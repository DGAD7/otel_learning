package main

import (
	"image/color"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/canvas"
	"fyne.io/fyne/v2/widget"
)

var (
	ledOnColor  = color.NRGBA{R: 0x2e, G: 0xcc, B: 0x40, A: 0xff} // green
	ledOffColor = color.NRGBA{R: 0xe0, G: 0x2b, B: 0x2b, A: 0xff} // red
)

type ledButton struct {
	widget.BaseWidget
	circle   *canvas.Circle
	onTapped func()
}

func newLedButton(onTapped func()) *ledButton {
	circle := canvas.NewCircle(ledOffColor)
	b := &ledButton{circle: circle, onTapped: onTapped}
	b.ExtendBaseWidget(b)
	return b

}

func (b *ledButton) SetOn(on bool) {
	if on {
		b.circle.FillColor = ledOnColor
	} else {
		b.circle.FillColor = ledOffColor
	}
}

func (b *ledButton) Tapped(*fyne.PointEvent) {
	b.onTapped()

}

func (b *ledButton) MinSize() fyne.Size {
	return fyne.NewSize(80, 80)
}

func (b *ledButton) CreateRenderer() fyne.WidgetRenderer {
	return widget.NewSimpleRenderer(b.circle)

}
