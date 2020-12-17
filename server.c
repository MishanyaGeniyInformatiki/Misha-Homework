#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define page_size 4096
#define N 256

void *p[3];
char dir[N];

void my_handler(int sig){
	free(p[0]);
	free(p[1]);
	free(p[2]);
	sprintf(dir, "%s/FIFO.fifo", dir);
	if (remove (dir) == -1){
		printf("remove error");
		exit(errno);
		}	
	exit(0);
}


int main(int argc, char * argv[]){

	signal(SIGINT, my_handler); // корректное завершение сервера по нажатию ctr+С
	
	if (getcwd(dir, N) == NULL) { // заносим в dir текущую директорию
		printf("getcwd error\n");
			exit(errno);
	}

	char path[N];
	sprintf(path, "%s/FIFO.fifo", dir);
	// в path храним путь к файлу FIFO.fifo

	if ( mkfifo (path, 0600) == -1){ // создает канал связи FIFO.fifo
		if (errno != EEXIST){
			printf("create fifo error\n");
			exit (errno);
		}
	}

	int fd_ser, fd_cl, fd;
	char *data = (char *) malloc (N*sizeof(char));
	p[0] = (void *) data; 
	char *file = (char *) malloc (N*sizeof(char));
	p[1] = (void *) file; 
	char * buf = (char *) malloc (page_size * sizeof (char));

	if (buf == NULL){
		printf("buf malloc error\n");
		exit(errno);
	}

	p[2] = (void *) buf;
	fd_ser = open (path, O_RDWR); // открывает канал FIFO.fifo для записи и чтения
		if ( fd_ser == -1 ){
			printf("open fifo1 error\n");
			exit (errno);
		}

	while (1){
		
		if( read(fd_ser, data, N)  == -1){ // считываю содержимое файла FIFO.fifo в дату
			printf("%s\n", data);
			printf("read from fifo error\n");
			exit (errno);
		}	

		file = strchr (data, ';');
		*file = '\0';
		file++; // в массиве file теперь записано название файла (argv[1])
				// а в массиве data теперь записан pid

		char new_path[N];
		sprintf(new_path, "%s/client%s.fifo",dir, data); // храню в new_path
		// путь к файлу client(id).fifo

		fd = open(file, O_RDONLY); // открываю файл (argv[1]) для чтения
		if ( fd == -1 ){
			printf("open file error\n");
			exit (errno);
		}

		if ( mkfifo (new_path, 0600) == -1){ // создает еще один client(id).fifo
			if (errno != EEXIST){
				printf("create fifo error\n");
				exit (errno);
			}
		}

		fd_cl = open(new_path, O_WRONLY); // открываю файл client(id).fifo для записи
				if ( fd_cl == -1 ){ 
					printf("open fifo error\n");
					exit (errno);
		}
		while (1){
			int rd = read(fd, buf, page_size); // читаю данный 
			// из файла (argv[1]) в buf
			if( rd  == -1){
				printf("read from file error\n");
				exit (errno);
			}	
			if (write(fd_cl, buf, rd) == -1){ // записываю прочитанные
			// из файла (argv[1]) данные в файл client(id).fifo 
				printf("write in fifo error\n");
				exit (errno);
			}
			if( rd < page_size){
				printf("reading end\n");
				break;
			}			
		}
		
		if (remove (new_path) == -1){
			printf("remove error");
			exit(errno);
		}		
		
	}
}