package main

import (
	"context"
	"fmt"
	"io"
	"log/slog"
	"os"
	"strings"

	"go.opentelemetry.io/contrib/bridges/otelslog"
	"go.opentelemetry.io/otel"
	"go.opentelemetry.io/otel/attribute"
	sdklog "go.opentelemetry.io/otel/sdk/log"
	"go.opentelemetry.io/otel/sdk/resource"
)

type lineExporter struct {
	w io.Writer
}

func (e lineExporter) Export(_ context.Context, records []sdklog.Record) error {

	for _, r := range records {
		var b strings.Builder
		fmt.Fprintf(&b, "%s %-5s [%s] %s",
			r.Timestamp().Format("15:04:05.000"),
			r.Severity().String(),
			r.InstrumentationScope().Name,
			r.Body().String(),
		)
		r.WalkAttributes(func(kv attribute.KeyValue) bool {
			fmt.Fprintf(&b, " %s=%s", kv.Key, kv.Value.String())
			return true
		})

		fmt.Fprintln(e.w, b.String())

	}
	return nil
}

func (lineExporter) ForceFlush(context.Context) error { return nil }
func (lineExporter) Shutdown(context.Context) error   { return nil }

func getLogger(name string) *slog.Logger {

	return otelslog.NewLogger(name)
}

func initLogging(ctx context.Context) (func(context.Context) error, error) {

	exporter := lineExporter{w: os.Stdout}

	res, err := resource.New(ctx,
		resource.WithTelemetrySDK(),
		resource.WithAttributes(attribute.String("service.name", "led-ui")),
	)
	if err != nil {
		return nil, err
	}

	provider := sdklog.NewLoggerProvider(
		sdklog.WithProcessor(sdklog.NewSimpleProcessor(exporter)),
		sdklog.WithResource(res),
	)

	otel.SetLoggerProvider(provider)

	slog.SetDefault(otelslog.NewLogger("led-ui"))
	return provider.Shutdown, nil

}
