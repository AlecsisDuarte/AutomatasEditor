#include "TextWindow.h"
#include <iostream>

using namespace Gtk;

TextWindow::TextWindow(BaseObjectType* cobject, const Glib::RefPtr<Builder>& refGlade) : Window(cobject), builder(refGlade){
    
}