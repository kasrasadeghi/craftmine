#define WEBVIEW_IMPLEMENTATION
#include "webview.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

typedef struct webview Webview;

void create_webview(Webview& w) {
  memset(&w, 0, sizeof(Webview));
  w.title = "Todo";
  w.url = "file:///home/kasra/projects/minecraft/docs/todoapp.html";
  w.width = 800;
  w.height = 600;
  w.resizable = true;
  w.external_invoke_cb = [](Webview* w, const char* data) {
    if (data == std::string("load")) {
      std::cout << "loading..." << std::endl;

      std::stringstream buffer;
      std::ifstream in {"todo.json"};
      buffer << in.rdbuf();
      const std::string jseval = "state = \"" + buffer.str() + "\"";
      webview_eval(w, jseval.c_str());
    } else {
      std::cout << "saving..." << std::endl;
      std::ofstream out {"todo.json"};
      out << data;
    } 
  };

  int r = webview_init(&w);
  if (r != 0) exit(r);
}

int main() {
  Webview w;
  create_webview(w);
  while (webview_loop(&w, 1) == 0) ;
  webview_exit(&w);
  return 0;
}