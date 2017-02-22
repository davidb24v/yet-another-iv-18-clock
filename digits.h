/*
 _            _  ___         _            _    
(_)_   __    / |( _ )    ___| | ___   ___| | __
| \ \ / /____| |/ _ \   / __| |/ _ \ / __| |/ /
| |\ V /_____| | (_) | | (__| | (_) | (__|   < 
|_| \_/      |_|\___/   \___|_|\___/ \___|_|\_\
                                               
Using the adaptor board by Awesomenesser:
https://oshpark.com/shared_projects/K4IOvS0o

Mappings from IV-18 wiring
(Page 2 of http://www.tube-tester.com/sites/nixie/dat_arch/IV-18.pdf)

+---------+--------+ 
|IV-18 Pin|MAX 6921|
+---------+--------+ 
|  1      |        |      IV-18:
|  2   .  |    9   |      .       Decimal Point
|  3   d  |   14   |      a-g     Segments "a"-"g"
|  4   c  |   10   |      1-9     Digit 1-9 Grid
|  5   e  |   15   |
|  6      |        |
|  7      |        |      MAX 6921:
|  8      |        |      x       Connected to OUTx       
|  9   g  |   13   |
| 10   b  |   11   |
| 11   f  |   12   |
| 12   a  |   16   |
| 13      |        |
| 14   9  |    1   |
| 15   1  |    3   |
| 16   3  |    5   |
| 17   5  |    6   |
| 18   8  |    7   |
| 19   7  |    4   |
| 20   6  |    2   |
| 21   4  |    0   |
| 22   2  |    8   |
+---------+--------+ 

Assuming "usual" 7-segment layout:

     --a--
    |     |
    f     b
    |     |
     --g--
    |     |
    e     c
    |     |
     --d--

That's enough information to define the digits we need to 
run the clock.

We're going to use SPI to send data to the MAX 6921 so we'll
use a uint32_t for storage:
               19    15       7       0
  +--------+--------+--------+--------+
  |XXXXXXXX|XXXX....|........|........|
  +--------+--------+--------+--------+
X = Don't Care

That's how the bits relate to the MAX 6921 output OUT0 - OUT19. In
theory that is. Unfortuntately though although in theory, practice
and theory are the same thing. In practice, they aren't.

After getting bizarre results using the above mappings I had to
do some simple experiments to deduce that my tube isn't a standard
IV-18. The following represents what I actually have.

+---------+--------+ 
|IV-18 Pin|MAX 6921|
+---------+--------+ 
|  1      |        |      IV-18:
|  2   a  |    9   |      .       Decimal Point
|  3   f  |   14   |      a-g     Segments "a"-"g"
|  4   b  |   10   |      1-9     Digit 1-9 Grid
|  5   g  |   15   |
|  6      |        |
|  7      |        |      MAX 6921:
|  8      |        |      x       Connected to OUTx       
|  9   e  |   13   |
| 10   c  |   11   |
| 11   d  |   12   |
| 12   .  |   16   |
| 13      |        |
| 14   2  |    1   |
| 15   4  |    3   |
| 16   6  |    5   |
| 17   7  |    6   |
| 18   8  |    7   |
| 19   5  |    4   |
| 20   3  |    2   |
| 21   1  |    0   |
| 22   9  |    8   |
+---------+--------+ 


First, start with the digit grids:
*/

#define DIG1    (1UL << 0)
#define DIG2    (1UL << 1)
#define DIG3    (1UL << 2)
#define DIG4    (1UL << 3)
#define DIG5    (1UL << 4)
#define DIG6    (1UL << 5)
#define DIG7    (1UL << 6)
#define DIG8    (1UL << 7)
#define DIG9    (1UL << 8)

// Now the segments
#define SEGA    (1UL <  9)
#define SEGB    (1UL < 10)
#define SEGC    (1UL < 11)
#define SEGD    (1UL < 12)
#define SEGE    (1UL < 13)
#define SEGF    (1UL < 14)
#define SEGG    (1UL < 15)
#define DP      (1UL < 16)

// Digits

// "0"
//   **a**
//  *     *
//  f     b
//  *     *
//   --g--
//  *     *
//  e     c
//  *     *
//   **d**
#define ZERO    (SEGA | SEGB | SEGC | SEGD | SEGE | SEGF)

// "1"
//   --a--
//  |     *
//  f     b
//  |     *
//   --g--
//  |     *
//  e     c
//  |     *
//   --d--
#define ONE     (SEGB | SEGC)

// "2"
//   **a**
//  |     *
//  f     b
//  |     *
//   **g**
//  *     |
//  e     c
//  *     |
//   **d**
#define TWO     (SEGA | SEGB | SEGG | SEGE | SEGD)

// "3"
//   **a**
//  |     *
//  f     b
//  |     *
//   **g**
//  |     *
//  e     c
//  |     *
//   **d**
#define THREE   (SEGA | SEGB | SEGG | SEGC | SEGD)

// "4"
//   --a--
//  *     *
//  f     b
//  *     *
//   **g**
//  |     *
//  e     c
//  |     *
//   --d--
#define FOUR    (SEGF | SEGG | SEGB | SEGC)

// "5"
//   **a**
//  *     |
//  f     b
//  *     |
//   **g**
//  |     *
//  e     c
//  |     *
//   **d**
#define FIVE    (SEGA | SEGF | SEGG | SEGC | SEGD)

// "6"
//   **a**
//  *     |
//  f     b
//  *     |
//   **g**
//  *     *
//  e     c
//  *     *
//   **d**
#define SIX     (SEGA | SEGF | SEGG | SEGE | SEGD | SEGC)

// "7"
//   **a**
//  |     *
//  f     b
//  |     *
//   --g--
//  |     *
//  e     c
//  |     *
//   --d--
#define SEVEN   (SEGA | SEGB | SEGC)

// "8"
//   **a**
//  *     *
//  f     b
//  *     *
//   **g**
//  *     *
//  e     c
//  *     *
//   **d**
#define EIGHT   (SEGA | SEGB | SEGC | SEGD | SEGE | SEGF | SEGG)

// "9"
//   **a**
//  *     *
//  f     b
//  *     *
//   **g**
//  |     *
//  e     c
//  |     *
//   **d**
#define NINE    (SEGA | SEGB | SEGC | SEGD | SEGF | SEGG)

// "-"
//
//   --a--
//  |     |
//  f     b
//  |     |
//   **g**
//  |     |
//  e     c
//  |     |
//   --d--
#define MINUS   SEGG

// " "
//
//   --a--
//  |     |
//  f     b
//  |     |
//   --g--
//  |     |
//  e     c
//  |     |
//   --d--
#define SPACE   0

// Define any further digits here....
//
//   --a--
//  |     |
//  f     b
//  |     |
//   --g--
//  |     |
//  e     c
//  |     |
//   --d--
