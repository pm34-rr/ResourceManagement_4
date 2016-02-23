/*!
 * \brief	Данная программа может быть использована для расчете
 *			числа сочетаний из n по k по формуле C(n, k) = n! / k!(n-k)!
 *
 * \author	Рогоза А. А.
 * \author	Романов С. А.
 * \date	23/02/2016
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>

const int MAX_INPUT = 13;

/*!
 * \brief Структура для хранения промежуточных записей в файле.
 */
struct Row
{
	__pid_t		id;
	uint64_t	factorial;
};

/*!
 * \brief Выводит сообщение об ошибке и аварийно завершает программу.
 * \param errorString Строка для вывода.
 */
void printError( const char * errorString )
{
	printf( errorString );
	printf( "\n" );
	exit( 1 );
}

/*!
 * \brief Считывает параметр для числа сочетаний.
 * \param outputStr Строка, соответствующая параметру.
 * \return В случае успеха чтения возвращает число.
 */
uint8_t readInputvalue( const char * outputStr )
{
	uint8_t n;
	printf( "Write " );
	printf( outputStr );
	printf( "\n" );
	scanf( "%" SCNu8, &n );
	if ( n > MAX_INPUT || n == 0 )
		printError( "Input value should be between 1 to 13" );

	return n;
}

/*!
 * \brief Вычисляет факториал числа и записывает в файл результат.
 * \param fd Дескриптор временного файла.
 * \param inputValue Число, факториал которого необходимо посчитать.
 */
void calculate( int fd, uint8_t inputValue )
{
	uint64_t factorial = 1;
	for ( int i = 2; i <= inputValue; ++i )
		factorial *= i;

	struct Row row;
	row.id			= getpid();
	row.factorial	= factorial;
	printf( "Hello from %d,", row.id );
	printf( "Fac = %" PRIu64, row.factorial );
	printf( "\n" );
	write( fd, &row, sizeof( row ) );
}

int main()
{
	//! Чтение параметра n
	uint8_t n	= readInputvalue( "n" );

	//! Создание временного файла
	const char * FILENAME = "temp";
	int fd		= open( FILENAME, O_CREAT | O_RDWR, 0777 );

	//! Создание первого процесса-потомка
	__pid_t id = fork();
	if ( id == 0 ) {
		calculate( fd, n );
		exit( 0 );
	}

	//! Чтение параметра k
	uint8_t k = readInputvalue( "k" );
	if ( k > n )
		printError( "k should be less or equal to n" );

	struct stat fileInfo;
	//! Создание второго процесса-потомка
	id = fork();
	if ( id == 0 ) {
		//! Ожидаем, пока не закончится запись во временный файл первым процессом-потомком.
		do
			fstat( fd, &fileInfo );
		while ( fileInfo.st_size != sizeof( struct Row ) );
		calculate( fd, k );
		exit( 0 );
	}

	uint8_t delta = n - k;
	//! Создание третьего процесса-потомка
	id = fork();
	if ( id == 0 ) {
		//! Ожидаем, пока не закончится запись во временный файл вторым процессом-потомком.
		do
			fstat( fd, &fileInfo );
		while ( fileInfo.st_size != (sizeof( struct Row ) * 2) );
		calculate( fd, delta );
		exit( 0 );
	}

	//! Ожидаем, пока не закончится запись во временный файл третьим процессом-потомком.
	do
		fstat( fd, &fileInfo );
	while ( fileInfo.st_size != (sizeof( struct Row ) * 3) );

	//! Сдвигаем указатель чтения на начало файла
	lseek( fd, 0, SEEK_SET );

	const int FACTORIALS_COUNT = 3;
	uint64_t factorials[FACTORIALS_COUNT];
	struct Row row;
	//! Считываем результаты вычисления промежуточных параметров
	for ( int i = 0; i < FACTORIALS_COUNT; ++i ) {
		read( fd, &row, sizeof( row ) );
		factorials[i] = row.factorial;
	}

	//! Закрываем временный файл
	close( fd );
	//! Удаляем временный файл
	remove( FILENAME );

	//! Вычисление и вывод результата
	uint64_t answer = factorials[0] / factorials[1] / factorials[2];
	printf( "Answer: %" PRIu64, answer );
	printf( "\n" );

	return 0;
}
