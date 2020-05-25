# Эмулятор процеccора с бинарной трансляцией в x86_64  
Задание 1 курса обучения в МФТИ. Проект состоит из нескольких частей. 
На вход программа принимает текстовый файл с кодом, написанным на диалекте
ассемблера, который придумал я. Далее этот код компилируется в бинарный файл, который
затем передается для выполнения эмулятору процессора.  
Также из этого бинарного файла можно получить RTE elf64-x86-64 файл для системы Linux.  
Для этого в проекте есть модуль бинарной трансляции, который еще и ускоряет программу.  
## Модули
 * `asm.cpp` - парсер файла на ассемблере. Принимает имя файла для компиляции как аргумент командной строки.  
 * `DisAsm.cpp` - диазассемблер используемого диалекта ассемблера.  
 * `cpu.cpp` - эмулятор процессора.  
 * `translator_x86_64.cpp` - бинарный транслятор из моего формата бинарных файлов в elf. 
 Принимает режим трансляции (текстовый или бинарный) параметрами командной строки.
 ## Компиляция и запуск
 За компиляцию отвечает скрипт `build.sh`.  
 `./build.sh [опция]`  
 * `-a` - скомпилировать asm.cpp  
 * `-d` - скомпилировать DisAsm.cpp  
 * `-c` - скомпилировать cpu.cpp  
 * `-b` - скомпилировать бинарный транслятор  
 * `-A` - скомпилировать все  
 За запуск отвечает скрипт `exec.sh`.  
 `./exec.sh [p1] [p2] [p3] [p4]`  
 1) [p1] :  
 * `-a` - запустить asm.cpp   
 * `-d` - запустить DisAsm.cpp  
 * `-c` - запустить cpu.cpp  
 * `-b` - запустить бинарный транслятор  
 * `-A` - запустить все  
 2) [p2] : имя исходного файла на ассемблере / режим бинарной трансляции  
 3) [p3] : режим бинарной трансляции / имя файла вывода для бинарного транслятора  
 4) [p4] : имя файла вывода для бинарного транслятора / ---  
## Оптимизация за счет бинарной трансляции  
Бинарная трансляция программы позволяет учкорить ее выполнение. Ниже приведены измерения времени работы для тестовой программы.
Для исследования использовалась программа вычисляющая факториал пяти 100 000 раз.  

```
чтоб не нести factorial (a)
вдоль ночных дорог
знаешь ли ты (a <= 1)
вдоль ночных дорог
не потеряй 1
не потеряй его и не сломай

не потеряй factorial (a - 1) * a
не потеряй его и не сломай

припев
вдоль ночных дорог
a теперь 5
i теперь 100000
res теперь 0
повторяю лишь (i > 0)
вдоль ночных дорог
res теперь factorial (a)
i теперь i - 1
не потеряй его и не сломай
сквозь слезы прошептала res
не потеряй его и не сломай
```
> Код на языке высочайшего уровня MAKSIM. Ссылка на репозиторий проекта https://github.com/Dimanteo/MAKSIM_language.  

Профилирование проводилось с помощью утилиты time  
`/usr/bin/time -p -a -o ProfileData.txt <executable>`  
 
| Виртуальный процессор, уровень оптимизации -O0 | Виртуальный процессор, уровень оптимизации -O3 | Бинарная трансляция |
|:---------------------:|:-------------------:|:------------------|
|         0.38 сек      |        0.09 сек     |         0.04      |

Таким образом, бинарная трансляция программы, ускоряет ее в **9.5** раз. 
