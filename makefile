svc:svc.c
	gcc svc.c -o svc

test:test.c
	gcc test.c -o test

working:working.c
	gcc working.c -o working

clean:
	-rm -rf svc test working