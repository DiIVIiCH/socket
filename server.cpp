#include "headers.h"
#include "data.h"
//объявление и имплементация класса сервера

class Server{
    private:
        int listener; //дескриптор сокета
        void send_time(int sock); //функция отправки серверного времени
        void send_random_string(int sock, int len); //функция отправки случайной строки
        void generate_random_string(unsigned char *s, int len); //генерирование случайной строки
    public:
        Server();
        Server(int domain, int type, int protocol = 0); 
        void set_socket(int domain, int type, int protocol = 0); //установка дескриптора listener
        void start_server(); //запуск сервера
        void stop_server(); //остановка сервера

};

Server::Server(){

    listener = -1;
}

Server::Server(int domain, int type, int protocol){
    set_socket(domain, type, protocol);
}

void Server::set_socket(int domain, int type, int protocol){
    listener=socket(domain, type, protocol); //устанавливаем дескриптор "слушателя"
}

void Server::start_server(){
    //работаем через AF_INET
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //пытаемся именовать сокет
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind"); //если всё плохо, то пишем ошибку и выходим
        exit(2);
    }
    //если всё хорошо, то задаём очередь запросов, в этом случае 1 
    listen(listener, 1);
    
    while(1)
    {   //создаём сокет для передачи
        int sock = accept(listener, NULL, NULL);

        if(sock < 0)
        {
            perror("accept"); //если что-то пошло не так, то пишем ошибку и выхоим
            exit(3);
        }

        
        int bytes_read; // сколько байт было прочитано
        struct pollfd pfd; //струкртура для poll
        pfd.fd = sock;  
        pfd.events = POLLIN | POLLHUP | POLLRDNORM;
        //ждем сообщение от клиента
        if (poll(&pfd, 1, -1) > 0){
            //что-то произошло
            packet buf; //объект пакета для получения информации
            bytes_read = recv(sock, &buf, sizeof(uint8_t), 0);  //читаем сообщение, возвращает количество прочитанных бит          
            if(bytes_read <= 0) break; //если пустое сообщение, значит ошибка, выходим
            printf("Command id:%i\n",buf.comm); //для себя в консоль пишем id команды от клиента
            //в зависимости от команды клиента, действуем
            switch(buf.comm){
                case 0:
                    send_time(sock);//отправляем клиенту время сервера
                    break;            
                case 1:
                    send_random_string(sock, 50); //отправляем клиенту случайную строку длиной 50 символов
                    break;
                case 2:
                    return; // выходим
            }
        }
    
        close(sock);//закрываем сокет

    }

}


void Server::send_time(int sock){

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );//получаем временную метку
    timeinfo = localtime ( &rawtime );//пишем её в tm структуру

    //на основе timeinfo формируем дату
    data datetime = {(timeinfo->tm_year%100),
                        timeinfo->tm_mon,
                        timeinfo->tm_mday,
                        timeinfo->tm_hour,
                        timeinfo->tm_min,
                        timeinfo->tm_sec};

    packet pack; //объект для отправки пакета данных
    pack.comm = 0; //команда
    uint64_t s; 
    memcpy(&s, &datetime,  sizeof(datetime)); //копируем всю дату в uint64_t
    pack.message = s;//пишем полученную дату в пакет
    send(sock, &pack, sizeof(packet), 0);//отправлем дату клиенту
}

void Server::send_random_string(int sock, int len){
    int size = len; //длина строки
    unsigned char str[size];//сторка
    generate_random_string(str, size);//генерим строку

    packet pack;//объект для пакета и команда
    pack.comm=1;

    int sended_bytes = 0, message_size = 5; //всего отправлено байт и размер одного сообщения
    //бъем строку на куски и отправляем, пока не отправим всё
    while(sended_bytes<size){
        unsigned char message_slice[5];
        for (int i=0; i<message_size; i++){
            message_slice[i] = str[i+sended_bytes];
        }
        uint64_t s;
        memcpy(&s, &message_slice,  sizeof(message_slice));
        pack.message = s;
        send(sock, &pack, sizeof(packet), 0);
        sended_bytes+=message_size;
    }

}

void Server::generate_random_string(unsigned char *s, int len){
   
    srand(time(NULL)); //устанавливаем сид рандома
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    //генерим строку нужной длины
    for (int i=0; i<len; i++){
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    s[len]=0;
    printf("%s\n",s); //выводим строку для сравнения с клиентом
}

int main()
{
    //создаем и стартуем сервер
    Server server = Server(AF_INET, SOCK_STREAM, 0);
    server.start_server();
    
    return 0;
}
