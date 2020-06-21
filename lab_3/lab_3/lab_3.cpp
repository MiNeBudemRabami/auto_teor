#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <fstream>

using namespace std;

struct line_struct
{
	deque <string> from;
	string by;
	deque <string> to;
};

deque <line_struct> arr;
deque <line_struct> arr2;

void read_file()
{
	line_struct trans_line;

	string temp;
	string from;
	string to;
	string by;

	for (;;)
	{
		cin >> temp;

		if (cin.eof())
		{
			break;
		}

		for (size_t i = 0; i < temp.size(); i++)
		{
			if (temp[i] != '-')
			{
				from.push_back(temp[i]);
			}
			else
			{
				for (size_t j = i + 2; j < temp.size(); j++)
				{
					to.push_back(temp[j]);
				}
				break;
			}
		}

		cin >> by;
		by.erase(0, 7);
		by.pop_back();
		by.pop_back();

		trans_line.by = by;
		trans_line.to.push_back(to);
		trans_line.from.push_back(from);
		arr.push_back(trans_line);

		trans_line.to.clear();
		trans_line.by.clear();
		trans_line.from.clear();
		temp.clear();
		by.clear();
		from.clear();
		to.clear();
	}
}

void glue_to()
{
	for (size_t i = 0; i < arr.size(); i++)
	{
		for (size_t j = i + 1; j < arr.size(); j++)
		{
			if ((arr[i].from[0] == arr[j].from[0]) and ((arr[i].by == arr[j].by)))
			{
				arr[i].to.push_back(arr[j].to[0]);
				sort(arr[i].to.begin(), arr[i].to.end());
				arr.erase(arr.begin() + j);
				--j;
			}
		}
	}
}

void merge_arr()
{
	for (;;)
	{
		if (arr2.empty())
		{
			break;
		}
		arr.push_back(arr2[0]);
		arr2.pop_front();
	}
}

void arr_sort()
{
	for (size_t i = 0; i < arr.size(); i++)
	{
		for (size_t j = 0; j < arr.size(); j++)
		{
			if (arr[i].from[0] < arr[j].from[0])
			{
				swap(arr[i], arr[j]);
			}
		}
	}
}

void new_line()
{
	for (size_t i = 0; i < arr.size(); i++)
	{
		for (size_t j = i + 1; j < arr.size(); j++)
		{
			for (size_t k = 0; k < arr[i].to.size(); k++)
			{
				if ((arr[i].to[k] == arr[j].from[0]) && (arr[i].to.size() > 1) && (arr[i].to != arr[j].from))
				{
					line_struct new_trans_line;
					new_trans_line.from = arr[i].to;
					new_trans_line.by = arr[j].by;
					new_trans_line.to = arr[j].to;
					arr2.push_back(new_trans_line);

					for (size_t m = 0; m < arr.size(); m++)
					{
						if ((arr[i].to == arr[m].to) && (m < arr.size() - 1))
						{
							continue;
						}
						else if ((arr[i].to == arr[m].to) && (m == arr.size() - 1))
						{
							//break;
						}

						if ((arr[j].from == arr[m].to))
						{
							break;
						}

						if ((arr[j].from != arr[m].to) && (m == arr.size() - 1))
						{
							arr.erase(arr.begin() + j);
							break;
						}
					}
				}
			}
		}
	}
}

void clear_repeats()
{
	for (size_t i = 0; i < arr.size(); i++)
	{
		for (size_t j = i + 1; j < arr.size(); j++)
		{
			if ((arr[i].from == arr[j].from) and (arr[i].to == arr[j].to) and (arr[i].by == arr[j].by))
			{
				arr.erase(arr.begin() + j);
			}
		}
	}
}

void print_dot(string name)
{
	ofstream fout(name);
	{
		fout << "digraph determination {" << endl;
		for (;;)
		{
			if (arr.empty())
			{
				break;
			}

			for (;;)
			{
				if (arr[0].from.empty())
				{
					break;
				}
				fout << arr[0].from[0];
				arr[0].from.pop_front();
			}

			fout << "->";

			for (;;)
			{
				if (arr[0].to.empty())
				{
					fout << ' ';
					break;
				}
				fout << arr[0].to[0];
				arr[0].to.pop_front();
			}

			fout << "[label=" << arr[0].by << "];" << ' ' << endl;

			arr.pop_front();
		}
		fout << "}" << endl;
	}
}

int main()
{
	read_file();

	clear_repeats();

	glue_to();

	new_line();

	merge_arr();

	new_line();

	clear_repeats();

	print_dot("result.dot");
}
