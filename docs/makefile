run: todoapp
	./todoapp

todoapp: todoapp.cpp webview.h
	g++ --std=c++17 todoapp.cpp -DWEBVIEW_GTK=1 `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0`  -o todoapp
	