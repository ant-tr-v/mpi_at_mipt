## Задачи по параллельному программированию

 _МФТИ весна 2018_

### 1) Первое задание
  a)_hello.cpp_ - все процессы выводят приветствие и свой номер, после чего завершаутся
  
  b)_harmonic.cpp_ - многопроцессное вычисление первых n членов гармонического ряда (0 процесс выводит свое время работы)
  
  c)_round.cpp_ - круговая синхронизация процессов (для наглядности, и предотвращения наложения вывода добавлена задержка)
  
  d)_exp.cpp_ - многопроцессное вычисление exp(x) с заданой точностью, _MyDouble.hpp_ - авторская длинная арифметика с плавающей точкой (только сложение, умножение и деление положительных чисел) (группа по умножению но не по сложению). 
  Запускается с помощбю python3 скрипта python.py с параметрами _степень_, _точность_, _число процессов_. Например ___python3 start.py 1 1000 4___ вычислит примерно 1000 знаков числа e на 4 процессорах
  
### 2) Лабораторная
  a)_timer.cpp_  - допуск (аналог _round.cpp_)
  
  b)_diff_eq.cpp_ - задача (принимает шаги сетки как параментры)
  
  само задание лежит на https://mipt.ru/drec/forstudents/study/studyMaterials/4kurs/lab1.pdf
