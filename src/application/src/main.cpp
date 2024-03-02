#include "application.hpp"

int main() {
     app::Application app;
     if(app.Initialize())
         app.Update();

    return 0;
}
