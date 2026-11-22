int calculate(int count, float factor);

int calculate(int count, float factor) {
    const int base = 10;

    {
        int base = 20;
        int temporary = base + count;
    }

    float result = count * factor;
    return count + base;
}

int main(void) {
    int count = 5;
    // float factor = 2.5;

    // int result = calculate(count, factor);

    return 0;
}