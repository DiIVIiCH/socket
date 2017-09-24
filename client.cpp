#include "headers.h"
#include "data.h"
//объявление и имплементация класса клиента

class Client{
    private:
        int sock;//дескриптор сокета
        bool is_sock();//проверка значения сокета
        void get_data_from_server(uint8_t command, int message_size, unsigned char *res);//запрос данных с сервера
    public:
        Client();
        Client(int domain, int type, int protocol = 0);
        void set_socket(int domain, int type, int protocol = 0);//установка дескриптора сокета
        data get_time();//запрос серверного времени
        std::string get_random_string();//запрос случайно строки от сервера
        void stop_server();//остановка сервера
};

Client::Client(){
    sock = -1;
}

Client::Client(int domain, int type, int protocol){
    set_socket(domain, type, protocol);
}

void Client::set_socket(int domain, int type, int protocol){
    sock = socket(domain, type, protocol); //устанавливаем дескриптор сокета
}

bool Client::is_sock(){
    return sock>0;
}

data Client::get_time(){
    int data_size = sizeof(data);//раземр структуры data
    unsigned char rawdate[data_size]; //объект для получения даты в виде байт
    get_data_from_server(0, data_size, rawdate );//запрос серверного времени

    data datetime;
    memcpy(&datetime, &rawdate, sizeof(data));//перевод "сырой" даты в структуру даты

    return datetime;
}

std::string Client::get_random_string(){
    int string_size = 50;	//длина строки
    unsigned char str[string_size];//объект для получения строки
    get_data_from_server(1, string_size, str );//запрос строки от сервера в "сыром" виде

    return std::string((char*)str,sizeof(str));
}

void Client::get_data_from_server(uint8_t command, int message_size, unsigned char *res){    
    //проверка всё ли нормально с сокетом
    if (is_sock()){        
        int recieved_bytes = 0; // количестве всех полученных байт
        struct sockaddr_in addr; // работаем через AF_INET               
        addr.sin_family = AF_INET;
        addr.sin_port = htons(3425);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        packet pack; // объект структры пакета данных
        pack.comm = command; // запись команды от клиента
        //пытаемся соединиться с сервером
        if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("connect");//если неудача, то пишем ошибку и выходим
            exit(2);
        }
        send(sock, &pack, sizeof(packet), 0);//если всё хорошо, то отправляем на сервер команду

        struct pollfd pfd;//структура для poll
        pfd.fd = sock; 
        pfd.events = POLLIN | POLLHUP | POLLRDNORM;
        //пока количество полученных байт меньше необходимого числа
        while(recieved_bytes<message_size){
            uint64_t response = 0;//переменная для записи сообщения от сервера
            unsigned char buf[5];//переменная для перевода сообщения от сервера в char[5]
            int recieved = 0;//количество полученных байт при последнем соединении
            //ждем ответа от сервера
            if (poll(&pfd, 1, 100) > 0){
                recieved =recv(sock, &pack, sizeof(packet), 0);//получаем ответ от сервера
                if (recieved<=0) break;//если сообщение пустое, то что-то пошло не так, выходим
                response = pack.message; // выводим сообщение от сервера в отдельную переменную
                memcpy(buf, (unsigned char*)&response,5);//переводим uint64_t в char[5]
                //пишем полуенное сообщение в переменную результата
                for (int i=recieved_bytes;i<recieved_bytes+recieved; i++){
                    res[i]=buf[i - recieved_bytes];
                }
                recieved_bytes += recieved-1;//увеличиваем количество полученных байт без учёта одного байта команды
            }            
        }
    }    
}



//останавливаем сервер
void Client::stop_server(){
    unsigned char s[1];
    get_data_from_server(2,0,s);
}

//вывод даты
void print_data(const data &datetime);

int main()
{
	//простенькое меню
    int command;
    std::cout<<"Choose command: 1 - get datetime, 2 - get random string, 3 - stop server:"<<std::endl;
    std::cin>>command;
    Client cl = Client(AF_INET, SOCK_STREAM);//создаём клиент
    switch(command){
        case 1:{        
                data datetime = cl.get_time(); //запрашиваем с сервера время
                print_data(datetime); // выводим его
                break;
            }
        case 2:{
                std::string random_string;//случайная строка
                random_string = cl.get_random_string(); //запрашвиаем случайную строку и выводим её
                for (int i=0; i<50;i++){
                    printf("%c",random_string[i]);
                }
                printf("\n");
                
                break;
            }
        case 3:
            cl.stop_server();//останавливаем сервер
            break;
    }
    return 0;
}


void print_data(const data &datetime){
    printf ( "Current year: %i \n", 2000+datetime.year);
    printf ( "Current month: %i \n", 1+datetime.month );
    printf ( "Current day: %i \n", datetime.day );
    printf ( "Current hour: %i \n", datetime.hour );
    printf ( "Current minute: %i \n", datetime.minute );
    printf ( "Current second: %i \n", datetime.second );
}
