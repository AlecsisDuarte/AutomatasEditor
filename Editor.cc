#include <iostream>
#include <gtkmm.h>
#include "TextWindow.h"

using namespace std;
using namespace Gtk;

int main(int argc, char *argv[]){
    cout << "Iniciando la aplicación" << endl;
    auto app = Gtk::Application::create(argc, argv, "org.gtkm.textide");
    Glib::RefPtr<Builder> builder;
    try{
        // builder = Builder::create_from_file("Views/editorWindow.glade");
        builder = Builder::create_from_file("Views/editorWindowNoBar.glade");
    }
    catch (const Glib::FileError &ex)
    {
        std::cerr << "FileError: " << ex.what() << std::endl;
        return 1;
    }
    catch (const Glib::MarkupError &ex)
    {
        std::cerr << "MarkupError: " << ex.what() << std::endl;
        return 1;
    }
    catch (const Gtk::BuilderError &ex)
    {
        std::cerr << "BuilderError: " << ex.what() << std::endl;
        return 1;
    }

    TextWindow *txtWindow = 0;
    builder->get_widget_derived("window", txtWindow);
    app->run(*txtWindow);   
    delete txtWindow;
    
    cout << "Aplicación terminada" << endl;
    return 0;
}