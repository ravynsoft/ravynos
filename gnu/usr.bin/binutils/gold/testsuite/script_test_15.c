int data[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

int extra[] __attribute__ ((section(".data.extra"))) = { 1, 2, 3, 4 };

int zeroes[1024] = {0};

int main(void)
{
  return 0;
}
