#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <fstream>
#include <cstdlib>
#include <ios>
#include <assert.h>
#include <stdlib.h>
#include <cstdio>


using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef map <string,int> MAP_type;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct S //структура токенов
{
    unsigned int number_str; //номер строки, в которой встретился токен
    int number_cmd; //номер команды, либо числовое значение
    char type_token; //тип токена,N-число,L-обращение к метке,U-использование метки,C-команда,R-регистр,X-неизвестная команда
} Token_inter;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct A//структура команд
{
    int number_cmd; //номер команды
    int val_arg; //номер аргумента
    char type_arg; //тип аргумента, N-число, R-регистр, L-метка, E-пустота
} SA;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
class ASM_h
{
    enum flags {CMD=01,LABEL_USAGE=02,LABEL_DECLARATION=04,REG=010,NUM=020,DEFAULT_FLAG=05};
    enum l_flags {NO_LABEL=-2,L_ERROR=-1};
    MAP_type MAP_h; //map функций
    MAP_type MAP_r; //map регистров
    MAP_type MAP_l; //map меток, конвертирующий имя в порядковое число
    vector <int> Labels;//таблица меток, конвертирующая порядок в номер токена-строки
    unsigned int labelnum; //количество меток, номер текущей метки
    vector <Token_inter> Token;//вектор структур токенов
    vector <SA> Command;//вектор команд
    bool success;
    public:
        ASM_h(void);
        ~ASM_h(void);
        string itoaa(int);
        int Get_token(string &,ifstream & f);
        bool Isnum(string &);
        void Translate(void);
        void Convert_labels(void);
        void Print_error(int&,int);
        bool Semantic(void);
        void Create_obj(void);
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void ASM_h::Convert_labels()
{
    int token_label;
    for (unsigned int i=0;i<Labels.size();i++)//идем по каждой метке
    {
        token_label=Labels[i];//запоминаем её номер токена
        if ((token_label==NO_LABEL)||(token_label==L_ERROR))//если в метке лежит флаг ошибки
            continue;//ковертируем следующую неошибочную метку
        for (unsigned int j=0;(j<Token.size())&&(j<(unsigned) token_label);j++)//бежим по токенам
        {
            if (Token[j].type_token!='C')//если не команда, то это операнд
                Labels[i]--;//и так пока мы не уберем все операнды/метки
        };
    };
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void ASM_h::Print_error(int& flague,int str)
{
    printf("Error: ");
    switch (flague)
    {
        case L_ERROR:
            printf("this label declared more than one time");
            break;
        case DEFAULT_FLAG:
            printf("excepted function or label declaration");
            break;
        case LABEL_USAGE:
            printf("excepted label usage");
            break;
        case NO_LABEL:
            printf("this label is not declared");
            break;
        case (CMD|REG):
            printf("excepted register or void");
            break;
        case (REG|NUM):
            printf("excepted register or number");
            break;
        case REG:
            printf("excepted register");
            break;
        case NUM:
            printf("excepted number");
            break;
        default:
            printf("unknown flag of argument");
            break;
    };
    printf(", line %d\n",str);
    success=false;
    flague=DEFAULT_FLAG;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ASM_h::Semantic(void)
{
    int flag=DEFAULT_FLAG;
    success=true;//успешен ли семантический анализ
    Convert_labels();
    SA Comds;
    for (unsigned int i=0;i<Token.size();i++)
    {
        switch (Token[i].type_token)
        {
            case 'C':
                if (flag&CMD)
                {
                    switch (Token[i].number_cmd)
                    {
                        case 2://принимает в качестве аргумента регистр,либо пустоту(pop)
                            flag=REG;
                            if (success)//если не было ошибок
                            {
                                Comds.number_cmd=Token[i].number_cmd;//пишем в вектор команд
                                Comds.val_arg=0;
                                Comds.type_arg='E';//на случай если не будет аргументов
                                if ((i==Token.size()-1)||(Token[i+1].type_token!='R'))//если следующий аргумент не существует или он-не регистр
                                {
                                    Command.push_back(Comds);
                                    flag=CMD;
                                };
                            };
                            break;
                        case 0: case 3: case 4: case 5: case 6: case 17: case 18: case 20: //определяем команды, не принимающие аргументов
                            flag=DEFAULT_FLAG;//потом идет функция/метка
                            if (success)//если до этого не было ошибок
                            {
                                Comds.number_cmd=Token[i].number_cmd;//пишем в вектор команд
                                Comds.val_arg=0;
                                Comds.type_arg='E';
                                Command.push_back(Comds);
                            };
                            break;
                        case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16://принимает в качестве аргумента метку
                            flag=LABEL_USAGE;
                            if (success)
                            {
                                Comds.number_cmd=Token[i].number_cmd;//пишем в вектор команд
                            };
                            break;
                        case 1: //принимает в качестве аргумента число или регистр
                            flag=NUM|REG;
                            if (success)
                            {
                                Comds.number_cmd=Token[i].number_cmd;
                            };
                            break;
                        case 7: case 8: case 19://принимает в качестве аргумента регистр
                            flag=REG;
                            if (success)
                            {
                                Comds.number_cmd=Token[i].number_cmd;
                            };
                            break;
                        default:
                            printf("Error: unknown command number, line %d\n",Token[i].number_str);
                            success=false;
                            flag=DEFAULT_FLAG;
                            break;
                    };
                }
                else
                {
                    Print_error(flag,Token[i].number_str);
                };
                break;
            case 'N':
                if (flag&NUM)//если ожидаем число
                {
                    if (success)//если не было ошибок, ибо если были, то нет смысла писать в вектор команд
                    {
                        Comds.val_arg=Token[i].number_cmd;
                        Comds.type_arg='N';
                        Command.push_back(Comds);
                        flag=DEFAULT_FLAG;
                    }
                }
                else
                {
                    Print_error(flag,Token[i].number_str);
                };
                break;
            case 'L':
                if (flag&LABEL_DECLARATION)//если она к месту
                {
                    if (Labels[Token[i].number_cmd]==L_ERROR)//если метка инициализируется более чем 1 раз
                        Print_error(flag=L_ERROR,Token[i].number_str);
                    flag=DEFAULT_FLAG;
                }
                else
                {
                    Print_error(flag,Token[i].number_str);
                };
                break;
            case 'U':
                if (flag&LABEL_USAGE)
                {
                    if (success)
                    {
                        if ((Labels[Token[i].number_cmd]!=L_ERROR)&&(Labels[Token[i].number_cmd]!=NO_LABEL))
                        {
                            Comds.val_arg=Labels[Token[i].number_cmd];
                            Comds.type_arg='L';
                            Command.push_back(Comds);
                            flag=DEFAULT_FLAG;
                        }
                        else if (Labels[Token[i].number_cmd]==L_ERROR)
                            Print_error(flag=L_ERROR,Token[i].number_str);
                        else
                            Print_error(flag=NO_LABEL,Token[i].number_str);
                    };
                }
                else
                {
                    Print_error(flag,Token[i].number_str);
                }
                break;
            case 'R':
                if (flag&REG)
                {
                    if (success)
                    {
                        Comds.val_arg=Token[i].number_cmd;
                        Comds.type_arg='R';
                        Command.push_back(Comds);
                        flag=DEFAULT_FLAG;
                    }
                }
                else
                {
                    Print_error(flag,Token[i].number_str);
                }
                break;
            case 'X':
                printf("Error: unknown token, line %d\n",Token[i].number_str);
                Print_error(flag,Token[i].number_str);
                break;
            default:
                printf("Error: unknown compilation error, line %d\n",Token[i].number_str);
                assert(0);
                break;
        };
    };
    return success;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
string ASM_h::itoaa (int n)
{
    char *s = new char[17];
    string w;
    if (n < 0)
    {
        n =-n;
        w = "-";
    }
    int i=0;
    do {
        s[i++]=n%10+'0';
        n-=n%10;
    } while (n /= 10);
    for (int j=i-1;j>=0;j--)
    {
        w+=s[j];
    }
    delete[] s;
    return w;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
int ASM_h::Get_token(string & s,ifstream & f)
{
    static int numstr=0;
    if (f.eof())//если мы прочитали все токены
        return EOF;
    else
    {
        while (f.peek()=='\n')//если встретился перевод на новую строку
        {
            f.get(); //удалем символ
            numstr++;//увеличиваем счетчик строк
        }
        f >> s;//извлекает из файла строку,игнорируя пробельные символы
        return numstr;
    }
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ASM_h::Isnum(string & s) //функция определяет только числа в десятичном формате(пока что)
{
    unsigned int it;
    if ((!isdigit(s[0]))&&(((s[0]!='+')&&(s[0]!='-'))||(s.size()==1))) //обработка возможного знака
        return false;
    for (it=1;it < s.size();it++)
    {
        if (!isdigit(s[it]))
            return false;
    }
    return true;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void ASM_h::Translate(void)
{
    string s;//буферная строка
    Token_inter One_token;
    labelnum=0;
    int nstr;
    ifstream f;
    f.open("input.txt",ios::in);
    if (f.fail())//если не удалось открыть файл
    {
        printf("input file open failed,aborting");
        assert(0);
    }
    MAP_h["end"]=0;//инициализируем map функций, пустая функция, сигнализирующая о необходимости выйти из программы
    MAP_h["push"]=1;//пушит число или значение в регистре в стек
    MAP_h["pop"]=2;//извлекает из стека число и удаляет его, либо помещает в регистр
    MAP_h["add"]=3;//суммирует два верхних числа в стеке и помещает результат в стек
    MAP_h["diff"]=4;//находит разность предпоследнего и последнего числа в стеке
    MAP_h["mul"]=5;//умножает ва верхних числа в стеке и помещает результат в стек
    MAP_h["div"]=6;//находит частное предпоследнего и последнего числа в стеке
    MAP_h["inc"]=7;//увеличивает значение в регистре на 1
    MAP_h["dec"]=8;//уменьшает значение в регистре на 1
    MAP_h["jmp"]=9;//прыгает на метку
    MAP_h["je"]=10;//прыгает на метку, если ZF=1
    MAP_h["jz"]=10;//ZF=1
    MAP_h["jne"]=11;//прыгает на метку, если ZF=0
    MAP_h["ja"]=12;//прыгает на метку, если CF=0 & ZF=0
    MAP_h["jnb"]=13;//CF=0
    MAP_h["jae"]=13;//CF=0
    MAP_h["jna"]=14;//CF=1 or ZF=1
    MAP_h["jbe"]=14;//CF=1 or ZF=1
    MAP_h["jb"]=15;//CF=1
    MAP_h["call"]=16;//вызов подпрограммы
    MAP_h["ret"]=17;//возврат из подпрограммы
    MAP_h["in"]=18;//ввод числа с экрана в стек
    MAP_h["out"]=19;//выводит число из регистра на экран
    MAP_h["cmp"]=20;//сравнивает два числа в стеке
    MAP_r["ax"]=1;//инициализируем map регистров
    MAP_r["bx"]=2;
    MAP_r["cx"]=3;
    MAP_r["dx"]=4;
    while ((nstr=ASM_h::Get_token(s,f))!=EOF)
    {
        One_token.number_str=nstr;
        if (MAP_h.find(s)!=MAP_h.end())  //если это функция
        {
            One_token.number_cmd=MAP_h[s];
            One_token.type_token='C';
            Token.push_back(One_token);
        }
        else if (Isnum(s))   //если это число
        {
            One_token.number_cmd=atoi(s.c_str());
            One_token.type_token='N';
            Token.push_back(One_token);
        }
        else if (MAP_r.find(s)!=MAP_r.end()) //если это регистр
        {
            One_token.number_cmd=MAP_r[s];
            One_token.type_token='R';
            Token.push_back(One_token);
        }
        else if (s[s.size()-1]==':') //если это объявление метки(1:)
        {
            s.pop_back(); //удаляем :
            if (MAP_l.find(s)==MAP_l.end()) //если метка еще не была объявлена(изначально мапа меток пустая)
            {
                MAP_l[s]=labelnum;
                Labels.push_back(Token.size());//запоминаем номер токена
                One_token.number_cmd=labelnum++;
            }
            else//если метка с таким именем уже встречалась
            {
                if  (Labels[MAP_l[s]]==NO_LABEL)//если мы до этого только пытались прыгнуть по этой метке
                    Labels[MAP_l[s]]=Token.size();
                else
                    Labels[MAP_l[s]]=L_ERROR;//ой, появились 2 метки с одинаковым именем
                One_token.number_cmd=MAP_l[s];
            };
            One_token.type_token='L';
            Token.push_back(One_token);
        }
        else if (s[0]==':') //если это использование метки(jmp :1)
        {
            s.erase(0,1); //удаляем :
            if (MAP_l.find(s)==MAP_l.end()) //если метка еще не была объявлена(изначально мапа меток пустая)
            {
                MAP_l[s]=labelnum;
                Labels.push_back(NO_LABEL);//куда прыгать мы пока не знаем, -2 является сигналом, что куда прыгать мы не знаем
                One_token.number_cmd=labelnum++;
            }
            else//если метка с таким именем уже встречалась
            {
                One_token.number_cmd=MAP_l[s];
            };
            One_token.type_token='U';
            Token.push_back(One_token);
        }
        else
        {
            One_token.number_cmd=0;
            One_token.type_token='X';
            Token.push_back(One_token);
        };
    }
    f.close();
    ofstream g("outASM1.txt"); //вот так вот мы создаем выходной файл outASM1.txt
    g.write("str",sizeof("str"));
    g.write("cmd",sizeof("cmd"));
    g.write("tkn\n",sizeof("tkn\n"));
    for (unsigned int i=0;i<Token.size();i++)
    {
        g.write(itoaa(Token[i].number_str).c_str(),sizeof(itoaa(Token[i].number_str)));
        g.write(itoaa(Token[i].number_cmd).c_str(),sizeof(itoaa(Token[i].number_cmd)));
        g.write(&(Token[i].type_token),sizeof(Token[i].type_token));
        g.write("\n",sizeof("\n"));
    }
    for (unsigned int i=0;i<Labels.size();i++)
    {
        g.write(itoaa(i).c_str(),sizeof(itoaa(i)));//хуйня to_string не работает,itoa тоже,надо найти альтернативу,спасибо стандартным библиотекам >:(
        g.write(itoaa(Labels[i]).c_str(),sizeof(itoaa(Labels[i])));
        g.write("\n",sizeof("\n"));
    }
    g.close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void ASM_h::Create_obj(void)
{
    FILE* h;
    h=fopen("Objectfile.txt","w");
    for (unsigned int i=0;i<Command.size();i++)
    {
        fprintf(h,"%d %d %c\n",Command[i].number_cmd,Command[i].val_arg,Command[i].type_arg);
    }
    fclose(h);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ASM_h::ASM_h(void)
{
    Translate();//тут мы переводим строки из файла в токены
    if(Semantic())//приступаем к следующему этапу-семантическому анализу, реализованному в отдельной функции
    {
        printf("Compilation finished\n");
        Create_obj();//создаем объектный файл
    }
    else
    {
        printf("Compilation failed\n");
    };
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ASM_h::~ASM_h(void)
{

};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    ASM_h();//пустой мэйн,чтобы компилятор не ругался
    return 0;
};
