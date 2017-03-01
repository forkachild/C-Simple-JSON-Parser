
test : main.c json.c
	gcc main.c json.c -o app.out && ./app.out
