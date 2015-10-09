int main(int argc, char *argv[])
{
    CriticalHandlerOn();

    setdisk(0);

    open("1", 1);

    printf("%d", CriticalErrorOccured());

    setdisk(2);

    return 0;
}
