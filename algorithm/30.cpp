#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>

int main(void)
{
    int n, cnt = 0;

    scanf("%d", &n);
    int input[n+1] = {0, };

    while (1) 

    printf("%d", cnt);
    return 0;
}

// 효율적으로 어떻게 짤 수 있지
// 규칙성을 찾아라
// 100개엔 3의 개수가 20개
// 200개엔 3의 개수가 40개
// 100개 단위일 때는 단위가 바뀌면 20개씩 증가한다
// 단, 백의 단위가 3일 경우엔 다르다 백의자리가 3일 경우엔 3의 개수가
// 계속 1개씩 더 늘어난다 주어진 숫자가 400이라 할 경우엔
// 300에서 399까지 3의 개수가 1개씩 더 늘어나 총 100가 더해지고
// 기존 20개씩 증가하던 규칙을 적용하니 180개가 된다
// 너무 막연하다 대체 몇까지 이런 식으로 구해야 하는 건가
// 효율성이 떨어진다 판단하는 지점을 어디로 삼아야 하나

// 너무 비효율적 기각

// 강의 참고
// 숫자들을 숫자 그대로 쓰면 너무 오래걸리니까 배열을 활용해보자

// 주어진 숫자의 길이를 구한다음 그 길이만큼 배열을 할당한다
// 그다음 숫자를 문자열로 뱐환해서 그 배열에 넣는다
// 혹은 배열에 0000으로 채운다음 입력받은 값까지 1의 자리를
// 계속 증가시키면서 3의 개수를 센다
// 이렇게 하면 시간 복잡도가 줄어드네 충분한가

아니면 그냥 확률로 계산하는 게 낫지 않나

자리수마다 3이 나오는 경우의 수를 구해서 곱집합으로 구해보면 되지 않나
주어진 수가 100이라 하면

1의자리 1개 = 3
10의 자리 10 * 1...?

경우의 수는 3가지식으로 구분해야 한다

입력 받은 숫자를 체크할 때 자리수의 숫자가 3보다 작냐 3과 같냐
3보다 크냐 이렇게 3가지 경우로 구분을 해야 한다

중복 계산을 해도 됨

십의자리에서 카운트 한게
백의 자리에서 카운트 되어도 되는 건가
헷갈리네

내일 다시 짜자

아 그러네 되네
그 자리수에 나오는 3만 세는 거니까 되는 거네
그렇네 그 자리수의 3만 세는 거
이 개념이 확실하게 잡혀야 하네 
나는 1234 라고하면
십의자리에서 3을 체크하면 1234 라는 숫자가 다시는 체크대상이 되어서 안 된다고 생각했는데
그게 아니었네 해당 자리에서 나오는 3의 개수를 세는 개념임
그런 식으로 접근하면 될 듯