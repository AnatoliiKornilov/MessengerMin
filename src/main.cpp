#include "app_context.hpp"
#include "app.hpp"
#include "config.hpp"

#include "httplib.h"

#include <exception>
#include <iostream>

int main() {
  try {
    Config config = Config::from_env();

    AppContext context(config);

    httplib::Server server;
    setup_routes(server, context);

    std::cout << "Server is listening on port " << config.port << std::endl;

    server.listen("0.0.0.0", config.port);
  } catch (const std::exception& exception) {
    std::cerr << "Fatal error: " << exception.what() << std::endl;

    return 1;
  }

  return 0;
}
