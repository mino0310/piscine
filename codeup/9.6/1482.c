#include <stdio.h>

int main(void)
{
	int n, m, i, j, k, arr[102][102], res = 0;

	scanf("%d %d", &n, &m);

	for (i = n; i >= -m; i--)
	{
		for (j = n; j >= 1; j--)
		{
			for (k = 1; k <= m; k++)
			{
				if (j - k == i)
					arr[j][k] = ++res;
			}
		}
	}

	for (i = 1; i <= n; i++)
	{
		for (j = 1; j <= m; j++)
		{
			printf("%d ", arr[i][j]);
		}
		printf("\n");
	}
}
