typedef char *array[10];

static array digits_names = {"zero", "one", "two", "three", "four",
			     "five", "six", "seven", "eight", "nine"};

void *foo (void)
{
  return digits_names;
}
