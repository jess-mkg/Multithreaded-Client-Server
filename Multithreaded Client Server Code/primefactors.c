#include <stdio.h>
//function to check for prime numbers
int checkprime(int num)
{
    if ((num == 2)||(num == 3))
        return (1);
    else
    {
        for (int i = 3; i <num; i = i + 2)
        {
            if (num % i == 0)
                return (0);
        }
        return 1;
    }
}

//function to find the prime factors
int primefactors(int num)
{
    int temp;
    temp = num;
    while(temp != 0 && temp % 2 == 0)
    {
        if (temp % 2 == 0)
        {
            printf("2 ");
            temp/=2;
        }
    }
    for (int k = 3; k <= temp && temp != 0; k = k + 2)
    {
        if (checkprime(k)==1)
        {
            while ((temp != 0) && (temp % k == 0))
            {
                printf("%d ",k);
                temp/=k;
            }
        }
    }
}
int main() {
    int a;
    a=10;
    if (a>1)
    {
        primefactors(a);
    }
    else
    {
        printf("Value is 1\n");
    }
}
