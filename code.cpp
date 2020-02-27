
#include "pch.h"
#include "omp.h"
//#include<bits/stdc++.h>
#include <thread>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <thread>
#define ll unsigned long long
//1 1 1 1 3 2 2 1 1 1 1 1 1 1 3 2 2 1 1 1 1 1 1 1 3 2 2 1 1 1 1 1 1 1 3 2 2 1 1 1
//srand 1205, n = rand() % 1000, int pool_len = 5000; int generations = 35;
// 123 sec non-par, 83 sec - par
using namespace std;

int n;
int k = 2;
vector<int> w;
const int threads = 4;
thread ts[threads];

void print_pool(vector<vector<int>> &pool, int pool_len) {
	for (int i = 0; i < pool_len; i++)
	{
		for (int j = 0; j < n; j++)
		{
			cout << pool[i][j] << " ";
		}
		cout << "\n";
	}
}

void get_individual(vector<int> &ind) {
	for (int i = 0; i < n; i++)
	{
		ind[i] = rand() % k;
	}
}

void get_pool(vector<vector<int>> &pool, int pool_len) {
	for (int i = 0; i < pool_len; i++)
	{
		get_individual(pool[i]);
	}
}

void individual_mutation(vector<int> &individual) {
	int rand_index = rand() % n;
	individual[rand_index] = (individual[rand_index] + (rand() % 2)*rand()) % k;
}

void pool_mutation(vector<vector<int>> &pool, int pool_len) {
	for (auto& ind : pool)
	{
		individual_mutation(ind);
	}
}

void cross(vector<int> &m1, vector<int> &m2, vector<int> &son) {
	for (int i = 0; i < n / 2; i++)
	{
		son[i] = m1[i];
	}
	for (int i = n / 2; i < n; i++)
	{
		son[i] = m2[i];
	}
}

bool comp(const vector<int> &m1, const vector<int> &m2) { //не работает для к
	int val1 = 0, val2 = 0, sum = 0;
	for (int i = 0; i < n; i++)
	{
		sum += w[i];
		val1 += w[i] * m1[i];
		val2 += w[i] * m2[i];
	}
	return abs(sum - 2 * val1) < abs(sum - 2 * val2);
}

void get_new_generation(vector<vector<int>> &pool, int pool_len) {
	for (int i = pool_len / 2; i < pool_len; i++)
	{
		int parent1_index = rand() % (pool_len / 2);
		int parent2_index = rand() % (pool_len / 2);
		cross(pool[parent1_index], pool[parent2_index], pool[i]);
	}
}

void f(vector<pair<vector<int>, int>> &XX, int j) {
	vector<int> v(k, 0);
	for (size_t i = 0; i < n; i++)
	{
		v[XX[j].first[i]] += w[i];
	}
	XX[j].second = 0;
	for (int i = 0; i < k - 1; i++)
	{
		XX[j].second += abs(v[i] - v[i + 1]);
	}
}

bool comp_(const pair<vector<int>, int> &m1, const pair<vector<int>, int> &m2) {
	return m1.second < m2.second;
}

void sort_pool2(vector<vector<int>> &pool, int pool_len) {
	sort(pool.begin(), pool.end(), comp);
}

void sort_pool(vector<vector<int>> &pool, int pool_len) {
	vector<pair<vector<int>, int>> XX(pool_len);
	for (int i = 0; i < pool_len; i++)
	{
		XX[i] = { pool[i], 0 };
	}
	for (int i = 0; i < pool_len; i++)
	{
		f(XX, i);
	}
	sort(XX.begin(), XX.end(), comp_);
	for (int i = 0; i < pool_len; i++)
	{
		pool[i] = XX[i].first;
	}
	cout << XX[0].second << " ";
}

vector<int> genetic_alg(vector<vector<int>> &pool, int pool_len, int generations) {
	for (int i = 0; i < generations; i++)
	{
		pool_mutation(pool, pool_len);
		sort_pool(pool, pool_len);
		get_new_generation(pool, pool_len);
	}
	return pool[0];
}

void print_solution(vector<int> &solution) {
	for (size_t i = 0; i < k; i++)
	{
		cout << i+1 << ": ";
		int sum = 0;
		for (int j = 0; j < n; j++)
		{
			if (solution[j] == i) {
				cout << w[j] << " ";
				sum += w[j];
			}
		}
		cout << "\n sum: " << sum << "\n";
	}
}

void initilization() {
	cin >> n;
	for (int i = 0; i < n; i++)
	{
		int wi;
		cin >> wi;
		w.push_back(wi);
	}
}

void parallel_pool_mutation(vector<vector<int>> &pool, int pool_len) {
	for (int i = 0; i < pool_len; i += threads)
	{
		for (int j = 0; j < threads; j++)
		{
			if (i + j < pool_len)
				ts[j] = thread(individual_mutation, ref(pool[i + j]));
		}
		for (auto& t : ts)
		{
			if (t.joinable()) t.join();
		}
	}
}

void parallel_pool_mutation2(vector<vector<int>> &pool, int pool_len) {
#pragma omp parallel shared(pool) num_threads(1)
	{
#pragma parallel for
		for (auto& ind : pool)
		{
			individual_mutation(ind);
		}
	}
}

void parallel_sort_pool(vector<vector<int>> &pool, int pool_len) {
	vector<pair<vector<int>, int>> XX(pool_len);
	for (int i = 0; i < pool_len; i++)
	{
		pair<vector<int>, int> xi = { pool[i], 0 };
		XX[i] = xi;
	}
	for (int i = 0; i < pool_len; i += threads)
	{
		for (int j = 0; j < threads; j++)
		{
			if (i + j < pool_len)
				ts[j] = thread(f, ref(XX), i + j);
		}
		for (auto& t : ts)
		{
			if (t.joinable()) t.join();
		}
	}
	sort(XX.begin(), XX.end(), comp_);
	for (int i = 0; i < pool_len; i++)
	{
		pool[i] = XX[i].first;
	}
	cout << XX[0].second << "\n";
}

void parallel_sort_pool2(vector<vector<int>> &pool, int pool_len) {
	vector<pair<vector<int>, int>> XX(pool_len);
	for (int i = 0; i < pool_len; i++)
	{
		XX[i] = { pool[i], 0 };
	}
#pragma omp parallel shared(XX) num_threads(4)
	{
#pragma parallel for
		for (int i = 0; i < pool_len; i++)
		{
			f(XX, i);
		}
	}
	sort(XX.begin(), XX.end(), comp_);
	for (int i = 0; i < pool_len; i++)
	{
		pool[i] = XX[i].first;
	}
}

vector<int> parallel_genetic_alg(vector<vector<int>> pool, int pool_len, int generations) {
	for (int i = 0; i < generations; i++)
	{
		pool_mutation(pool, pool_len);
		//sort_pool(pool, pool_len);
		//parallel_pool_mutation(pool, pool_len);
		parallel_sort_pool(pool, pool_len);
		get_new_generation(pool, pool_len);
	}
	return pool[0];
}

void random_initalization() {
	n = rand() % 100;
	cout << "n = " << n << "\n";
	for (size_t i = 0; i < n; i++)
	{
		w.push_back(rand() % 100);
	}
}

int main()
{
	srand(1205);
	//initilization();
	random_initalization();
	cout << "количество кучек: \n";
	cin >> k;
	int prog;
	cout << "Parallel? 0 - no, 1 - yes\n";
	cin >> prog;
	double start_time = clock();
	int pool_len = 500;
	int generations = 35;
	vector<vector<int>> pool = vector<vector<int>>(pool_len, vector<int>(n));
	get_pool(pool, pool_len);
	vector<int> solution;
	if (prog) {
		solution = parallel_genetic_alg(pool, pool_len, generations);
	}
	else {
		solution = genetic_alg(pool, pool_len, generations);
	}
	cout << "\n";
	print_solution(solution);
	cout << (clock() - start_time) / 1000 << " sec";
}