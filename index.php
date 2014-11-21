<?php
/**
 * Тестовое задание для кандидатов на должность PHP-разработчика
 *
 * Файл представляет собой шаблон для выполнения тестового заданияю Все объявленные
 * методы должны быть реализованы непосредственно здесь. Создание дополнительных
 * собственных методов допускается.
 *
 * Код должен соотвестсовать стандартам кодирования Zend Framework (http://framework.zend.com/manual/ru/coding-standard.html)
 * и работать без генерации предупреждений при включенном режиме error_reporting=E_ALL.
 *
 * После выполнения всех задач файл должен быть переименован в следующий формат:
 * <Date>-<LastName>.php
 *
 * Например, 20131216-Ivanov.php
 *
 * ВНИМАНИЕ!
 *     - На выполнение задания вы не должны тратить более двух-трех дней.
 *     - Файл должен содержать только класс.
 */
class PetrosoftCandidate {
	
   	public $task4_array = array();	
    /**
     * Задание 1
     *
     * Во входной строке переставить слова в обратном порядке. Например строка
     * «one two three four» должна быть преобразована в «four three two one».
     *
     * @param  string $str    Входная строка
     * @return string         Строка с переставленными словами
     */
    public function task1($str) {
	$list = explode(" ", $str);
    $d_list = array_reverse($list);
    $d_string = implode(" ", $d_list);
	return $d_string;
    }


    /**
     * Задание 2
     *
     * В неупорядоченном массиве целых положительных чисел определить положение
     * и длину наиболее длинной группы, представляющей собой перестановку элементов
     * отрезка натурального ряда чисел.
     *
     * Элементы массива
     * 6, 8, 5, 7, 18, 21, 20, 16, 19, 17
     *
     * В нем перестановками являются
     * 6, 8, 5, 7
     * и
     * 18, 21, 20, 16, 19, 17
     *
     * Вторая группа является наибольшей. Индекс первого элемента второй группы равен 4,
     * длина группы равна 6. Метод должен вернуть
     *
     * array(4, 6)
     *
     * @param  array   $list  Входной массив
     * @return array          Массив из двух элементов. Первый — индекс первого элемента
     *                        самой длинной группы, второй — ее длина.
     */
    public function task2($list)
	{
		$length_list = count($list);
		
	  	for ($i = $length_list; $i > 0; $i--)
		{  
			for ($n = 0; $n <= $length_list - $i; ++$n)
		 	{ 
				$need_array = array_slice($list, $n, $i);
				if ($this->task2_helper($need_array))
		   		{  
		    		return array($n, $i);
				}
			}
		 }
	}
	
	private function task2_helper($list)
	{
		sort($list);
	 	$m = $list[0];
	 	for ($i = 1;$i <= sizeof($list) - 1;$i++)
	 	{
	 		if ($list[$i] != ($m + 1))
		 	{
				 return false; 
		 	}
		 	$m = $list[$i];
	  	}
	  	return true;
	}

    /**
     * Задание 3
     *
     * Элементы, расположенные на периметре прямоугольной матрицы, отсортировать по часовой
     * стрелке в порядке возрастания, начиная с элемента, расположенного в верхнем левом
     * углу матрицы.
     *
     * Например, для входной матрицы:
     * 1 2 3 4
     * 5 6 7 8
     * 9 0 1 2
     *
     * Должен быть возвращен результат:
     * 0 1 1 2
     * 9 6 7 2
     * 8 5 4 3
     *
     * @param  array $matrix  Прямоугольная матрица
     * @return array          Матрица с отсортированными по периметру элементами
     */
    public function task3($matrix) {
        $matrix_d = array();
		$m = sizeof($matrix);
		$n = sizeof($matrix[0]);
		$k = $m - 2;
		
		$perimeter_array = self::task3_helper_1($matrix, $m, $n);
		sort($perimeter_array);
							
		for ($i = 0;$i < $n;$i++)
		{
			$matrix_d[0][$i] = $perimeter_array[$i];
			unset($perimeter_array[$i]);
		}
		sort($perimeter_array);
		
		for ($i = 1;$i <= $k;$i++)
		{
			$matrix_d[$i][0] = $perimeter_array[sizeof($perimeter_array) - 1];
			unset ($perimeter_array[sizeof($perimeter_array) - 1]);
			for ($z = 1;$z <= ($n - 1);$z++)
			{
				$matrix_d[$i][$z] = 0;
			}	
			$matrix_d[$i][$n-1] = $perimeter_array[0];
			unset($perimeter_array[0]);
			sort($perimeter_array);
		}
		
		sort($perimeter_array);
		
		$perimeter_array = array_reverse($perimeter_array);
		$matrix_d[$m - 1] = $perimeter_array;
		
		$matrix = self::task3_helper_2($matrix, $matrix_d, $m, $n);
		return $matrix;		
    }

	private function task3_helper_1($matrix, $m, $n)
	{
		$perimeter_array = array();
				
		for ($i = 0;$i < sizeof($matrix);$i++)
		{
			for ($j = 0;$j < sizeof($matrix[$i]);$j++)
			{
				if ($i == 0)
				{
					$perimeter_array[] = $matrix[$i][$j];
					continue;
				}
				elseif ($i == $n - 1)
				{
					$perimeter_array[] = $matrix[$i][$j];
					continue;
				}
				elseif ($j == 0)
				{
					$perimeter_array[] = $matrix[$i][$j];
					continue;
				}
				elseif ($j == $n - 1 )
				{
					$perimeter_array[] = $matrix[$i][$j];
					continue;
				}
				elseif ($i == $m - 1 )
				{
					$perimeter_array[] = $matrix[$i][$j];
					continue;
				}
			}
		}
		
	    return $perimeter_array;
	}
	
    private function task3_helper_2($matrix, $matrix_d, $m, $n)
	{
		for ($i = 0;$i < sizeof($matrix);$i++)
		{
			for ($j = 0;$j < sizeof($matrix[$i]);$j++)
			{
				if ($i == 0)
				{
					$matrix[$i][$j] = $matrix_d[$i][$j];
					continue;
				}
				elseif ($i == $n - 1)
				{
					$matrix[$i][$j] = $matrix_d[$i][$j];
					continue;
				}
				elseif ($j == 0)
				{
					$matrix[$i][$j] = $matrix_d[$i][$j];;
					continue;
				}
				elseif ($j == $n - 1 )
				{
					$matrix[$i][$j] = $matrix_d[$i][$j];
					continue;
				}
				elseif ($i == $m - 1 )
				{
					$matrix[$i][$j] = $matrix_d[$i][$j];;
					continue;
				}
			}
		}
		return $matrix;
	}	

    /**
     * Задание 4
     *
     * Сформировать одномерный массив, получающийся при чтении квадратной матрицы по спирали, начиная
     * с верхнего левого элемента матрицы (против часовой стрелки).
     *
     * Например, для входной матрицы:
     *  1  2  3  4
     *  5  6  7  8
     *  9 10 11 12
     * 13 14 15 16
     *
     * Должен быть возвращен результат:
     * 1 5 9 13 14 15 16 12 8 4 3 2 6 10 11 7
     *
     * @param  array $matrix  Входная квадратная матрица
     * @return array          Одномерный массив
     */
    public function task4($matrix) {
       self::task4_helper($matrix);
	   return $this->task4_array;
    }
	
	private function task4_helper($matrix)
	{
		$n = sizeof($matrix) - 1;
		
		for ($i = 0;$i <= $n;$i++)
		{
			$this->task4_array[] = $matrix[$i][0];
			unset($matrix[$i][0]);
		}
		for ($i = 1;$i <= $n;$i++)
		{
			$this->task4_array[] = $matrix[$n][$i];
			unset($matrix[$n][$i]);
		}
		for ($i = $n - 1;$i >= 0;$i--)
		{
			$this->task4_array[] = $matrix[$i][$n];
			unset($matrix[$i][$n]);
		}
		for ($i = $n - 1;$i > 0;$i--)
		{
			$this->task4_array[] = $matrix[0][$i];
			unset($matrix[0][$i]);
		}
		for ($i = 0;$i <= $n;$i++)
		{
			if (sizeof($matrix[$i]) == NULL)
			{
				unset($matrix[$i]);
			}
			else
			{
				$matrix[$i] = array_values($matrix[$i]);
			}
		}
		$matrix = array_values($matrix);
		if (sizeof($matrix) > 1)
		{
			self::task4_helper($matrix);
		}
		else
		{
			if (isset($matrix[0][0]))
			{
				$this->task4_array[] = $matrix[0][0];
			}	
		}
		
	}
}
$obj = new PetrosoftCandidate;
//print_r($obj->task1('The first Petrosoft test task'));
//print_r($obj->task2(array(6,8,5,7,18,21,19,16,19,17)));
/*print_r($obj->task3(array(array(1,2,3,4),
				  array(5,6,7,8),
				  array(9,0,1,2) )));*/
/*print_r($obj->task4(array(array(73,34,2,45,54),
				  array(6,93,77,23,67),
				  array(73,12,37,8,64),
				  array(42,49,73,87,4),
				  array(7,2,18,2,89))));*/	
/*function task2($list)
{ 
	 //Пусть длина исходного массива N. Будем последовательно искать в нём перестановки длины N, N-1, N-2 и т.д.
		$listLength = count($list);
	  	for ($i = $listLength; $i > 0; $i--)
		{  //ищем перестановки длины $i перебирая подмассивы
		echo $i.' - ';
	  		for ($offset = 0; $offset <= $listLength - $i; ++$offset)
		 	{ 
				echo $offset.'/';
				$candidate = array_slice($list, $offset, $i);
				print_r($candidate);echo '</br>';
		  		if (isSwap($candidate))
		   		{  //перестановка найдена!
		    		return array($offset, $i);
				 }
			}
		 }
	 
}	   
		
function isSwap($list) 
{
	 sort($list);
	 $previousValue = $list[0] - 1;//  начальное значение этой переменной задаём сами
	
	 foreach ($list as $value)
	 {
	 	 if ($value != ($previousValue + 1))
		 {
			 return false; 
		 }
		 $previousValue = $value;
	  }
	  return true;
}
			 					   
print_r (task2(array(6, 8, 5, 7, 18, 21, 20, 16, 19, 17)));*/				   
$a = array(1,2,3);
$b = array(4,5,6);
var_dump($a.$b);
