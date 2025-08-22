.PHONY: clean

lyra_plugin.so: lyra_plugin.o
	gcc build/lyra_plugin.o -shared -o build/lyra_plugin.so

lyra_plugin.o:
	gcc -I include/ -c -fPIC src/lyra_plugin.c -o lyra_plugin.o
	mv -v lyra_plugin.o build/

clean:
	rm -vf build/