#include "application.h"

int main() {
    app::Application app;

    if(app.Initialize())
        app.Update();    

    return 0;
}