all:
	gcc mysql.c -o mysql $(mysql_config --libs --cflags)
