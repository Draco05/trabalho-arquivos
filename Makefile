all:
	gcc -o programaTab *.c
run:
	./programaTab
copy:
	cp ./casos/pratico1/entrada/*.bin ./
clean:
	rm *.bin programaTab arquivos.zip
zip:
	zip -r arquivos.zip *.c *.h Makefile