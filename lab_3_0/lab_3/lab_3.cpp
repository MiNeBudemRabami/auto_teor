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

	for (;;)
	{
		string from = "";
		string to = "";

		cin >> from >> trans_line.by >> to;

		trans_line.from.push_back(from);
		trans_line.to.push_back(to);


		if (cin.eof())
		{
			break;
		}

		arr.push_back(trans_line);
		trans_line.from.clear();
		trans_line.to.clear();
	}
}

void print_info_line()
{
	cout << "from" << '\t' << "by" << '\t' << "to" << endl;
}

void print_output()
{
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
				cout << '\t';
				break;
			}
			cout << arr[0].from[0];
			arr[0].from.pop_front();
		}

		cout << arr[0].by << '\t';

		for (;;)
		{
			if (arr[0].to.empty())
			{
				break;
			}
			cout << arr[0].to[0];
			arr[0].to.pop_front();
		}
		cout << endl;
		arr.pop_front();
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

void new_line()
{
	for (size_t i = 0; i < arr.size(); i++)
	{
		for (size_t j = 0; j < arr.size(); j++)
		{
			for (size_t k = 0; k < arr[i].to.size(); k++)
			{
				if ((arr[i].to[k] == arr[j].from[0]))
				{
					line_struct new_trans_line;
					new_trans_line.from = arr[i].to;
					new_trans_line.by = arr[j].by;
					new_trans_line.to = arr[j].to;
					arr2.push_back(new_trans_line);

					for (size_t m = j + 1; m < arr.size(); m++)
					{
						for (size_t n = 0; n < arr[m].to.size(); n++)
						{
							if ((arr[j].from[0] != arr[m].to[n]) and (m == arr.size() - 1))
							{
								arr.erase(arr.begin() + j);
								if (j > 0)	j--;
								if (i > 0)	i--;
								break;

							}
						}
					}
				}
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

void print_dot()
{
	ofstream fout("graph.dot");
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
					fout << '\t';
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
					fout << '\t';
					break;
				}
				fout << arr[0].to[0];
				arr[0].to.pop_front();
			}

			fout << "[label = " << arr[0].by << "];" << '\t' << endl;

			arr.pop_front();
		}
		fout << "}" << endl;
	}
}

int main()
{
	read_file();

	glue_to();

	new_line();

	merge_arr();

	clear_repeats();

	print_dot();
}
