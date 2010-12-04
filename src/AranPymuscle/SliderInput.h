#ifndef __SLIDER_INPUT_
#define __SLIDER_INPUT_
class SliderInput;

class SliderInputFloatPart : public Fl_Float_Input {
public:
  SliderInputFloatPart(SliderInput *s, int X,int Y,int W,int H,const char *l = 0)
    : Fl_Float_Input(X,Y,W,H,l), slider(s) {}
  int handle(int event);
private:
  SliderInput *slider;
};

class SliderInput : public Fl_Group {
  Fl_Float_Input *input;
  Fl_Slider    *slider;

  // CALLBACK HANDLERS
  //    These 'attach' the input and slider's values together.
  //
  void Slider_CB2();

  static void Slider_CB(Fl_Widget *w, void *data);

  void Input_CB2();
  static void Input_CB(Fl_Widget *w, void *data);

public:
  // CTOR
  SliderInput(int x, int y, int w, int h, const char *l=0);

  // MINIMAL ACCESSORS --  Add your own as needed
  double value() { return slider->value(); }
  void value(double val) { slider->value(val); Slider_CB2(); }
  void minumum(double val) { slider->minimum(val); }
  double minumum() { return slider->minimum(); }
  void maximum(double val) { slider->maximum(val); }
  double maximum() { return slider->maximum(); }
  void bounds(double low, double high) { slider->bounds(low, high); }

  friend class SliderInputFloatPart;
};

#endif // #ifndef __SLIDER_INPUT_