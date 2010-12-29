#include "BwPch.h"
#include "SliderInput.h"
// sliderinput -- simple example of tying an fltk slider and input widget together
// 1.00 erco 10/17/04

void SliderInput::Slider_CB2()
{
  static int recurse = 0;
  if ( recurse ) { 
    return;
  } else {
    recurse = 1;
    char s[128];
    sprintf(s, "%.7lf", slider->value());
    // fprintf(stderr, "SPRINTF(%d) -> '%s'\n", (int)slider->value(), s);
    input->value(s);    // pass slider's value to input
    recurse = 0;
  }
}

void SliderInput::Slider_CB( Fl_Widget *w, void *data )
{
  ((SliderInput*)data)->
    Slider_CB2();
}

void SliderInput::Input_CB2()
{
  static int recurse = 0;
  if ( recurse ) {
    return;
  } else {
    recurse = 1;
    double val = 0;
    if ( sscanf(input->value(), "%lf", &val) != 1 ) {
      val = 0;
    }
    // fprintf(stderr, "SCANF('%s') -> %d\n", input->value(), val);
    val = slider->clamp(val);
    char s[128];
    sprintf(s, "%.7lf", val);
    input->value(s);
    slider->value(val);         // pass input's value to slider
    recurse = 0;
  }
}

void SliderInput::Input_CB( Fl_Widget *w, void *data )
{
  ((SliderInput*)data)->
    Input_CB2();
}

SliderInput::SliderInput( int x, int y, int w, int h, const char *l/*=0*/ ) : Fl_Group(x,y,w,h,l)
{
  int in_w = 120;
  int in_h = h;

  input  = new SliderInputFloatPart(this, x, y, in_w, in_h);
  input->callback(Input_CB, (void*)this);
  input->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);

  slider = new Fl_Slider(x+in_w, y, w - 20 - in_w, in_h);
  slider->type(FL_HORIZONTAL);
  slider->callback(Slider_CB, (void*)this);

  bounds(0, 10);     // some usable default
  value(0);          // some usable default
  end();             // close the group
}

int SliderInputFloatPart::handle( int event )
{
  if(event == FL_UNFOCUS) {
    slider->Input_CB2();
  }
  return Fl_Float_Input::handle(event);
}
