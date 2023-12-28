typedef char *array[9];

static array digits_names = {"one", "two", "three", "four",
			     "five", "six", "seven", "eight", "nine"};

void *bar (void)
{
  return digits_names;
}
