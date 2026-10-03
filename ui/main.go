package main

import (
	"context"
	"fmt"
	"log"
	"os"

	grpcService "ui/grpc/generated"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"
	"go.opentelemetry.io/otel/attribute"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func dialLedControl(target string) (grpcService.LedControlClient, func() error, error) {

	conn, err := grpc.NewClient(target,
		grpc.WithTransportCredentials(insecure.NewCredentials()))

	if err != nil {
		return nil, nil, fmt.Errorf("dialing led_control at %s: %w", target, err)
	}

	return grpcService.NewLedControlClient(conn), conn.Close, nil

}

func envOrDefault(name, defaultValue string) string {
	if v := os.Getenv(name); v != "" {
		return v
	}
	return defaultValue
}

func main() {

	ctx := context.Background()

	shutdownLogging, err := initLogging(ctx)
	if err != nil {
		fmt.Fprintf(os.Stderr, "failed to init otel logging: %v\n", err)
		os.Exit(1)
	}
	defer shutdownLogging(context.Background())

	logger := getLogger("main")
	logger.Info("This is the UI of the OTEL learning projects. This provides the UI for the led project")

	tracer := getTracer("main")

	ledControlAddress := envOrDefault("LED_CONTROL_ADDRESS", "localhost:50051")
	client, closeConn, err := dialLedControl(ledControlAddress)

	if err != nil {
		log.Fatalf("failed to connect to led_control grpc server: %v", err)
	}
	defer closeConn()

	uiApp := app.NewWithID("io.github.otel_learning.ui")
	window := uiApp.NewWindow("Led Control")

	statusLabel := widget.NewLabel("LED is Off")
	totalLAbel := widget.NewLabel("Switched On 0 times")

	ledOn := false
	var led *ledButton

	led = newLedButton(func() {

		tapCtx, span := tracer.Start(ctx, "let button tap")
		defer span.End()

		requestedState := !ledOn
		span.SetAttributes(attribute.Bool("led.requestedState", requestedState))
		resp, err := client.SetLed(tapCtx, &grpcService.SetLedRequest{On: requestedState})

		if err != nil {

			statusLabel.SetText(fmt.Sprintf("error calling led_control: %v", err))
			logger.ErrorContext(tapCtx, "error calling led_control gRPC service", "error", err)
			return
		}
		ledOn = resp.GetOn()
		logger.InfoContext(tapCtx, "LED state changed", "on", ledOn)

		led.SetOn(ledOn)
		if ledOn {
			statusLabel.SetText("LED is On")
		} else {
			statusLabel.SetText("LED is Off")
		}
		totalLAbel.SetText(fmt.Sprintf("Switched On %d times", resp.GetTotalTimesSwitchOn()))
	})

	window.SetContent(container.NewVBox(
		widget.NewLabel("LED Control"),
		container.NewCenter(led),
		statusLabel,
		totalLAbel,
	))

	window.Resize(fyne.NewSize(320, 240))
	window.ShowAndRun()
}
