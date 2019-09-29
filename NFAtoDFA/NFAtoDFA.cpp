// NFAtoDFA.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

void StringSetUnion(set<string> &targ, set<string> const& add)
{
	for (auto elem : add)
	{
		targ.insert(elem);
	}
}

struct NFA
{
	vector<set<string>> states;
	vector<string> inputs;
	string startState;
	set<string> acceptStates;
	vector<vector<set<string>>> table;

	static const size_t BAD_INDEX = SIZE_MAX;

	size_t GetStateIndex(set<string> const& state) const
	{
		for (size_t i = 0; i < states.size(); ++i)
		{
			if (state == states[i])
			{
				return i;
			}
		}

		return BAD_INDEX;
	}

	size_t GetInputIndex(string const& str) const
	{
		for (size_t i = 0; i < inputs.size(); ++i)
		{
			if (str == inputs[i])
			{
				return i;
			}
		}

		return BAD_INDEX;
	}

	void InsertState(set<string> const& srcState, string const& input, set<string> const& targState)
	{
		const size_t srcStateIndex = GetStateIndex(srcState);
		const size_t inputIndex = GetInputIndex(input);
		assert(srcStateIndex != BAD_INDEX && inputIndex != BAD_INDEX);
		StringSetUnion(table[srcStateIndex][inputIndex], targState);
	}

	void AddPowersetState(set<string> const& powerState)
	{
		states.push_back(powerState);

		const vector<set<string>> emptyNode(inputs.size());
		table.push_back(emptyNode);

		for (auto state : powerState)
		{
			const size_t stateInd = GetStateIndex({ state });
			assert(stateInd != BAD_INDEX);

			for (size_t in = 0; in < inputs.size(); ++in)
			{
				StringSetUnion(table.back()[in], table[stateInd][in]);
			}
		}
	}
};

vector<string> GetInBraceParts(string const& str, char lbrace, char rbrace)
{
	vector<string> result;
	size_t startFrom = 0;

	while (startFrom < str.length())
	{
		const size_t begin = str.find(lbrace, startFrom);
		const size_t end = str.find(rbrace, startFrom);
		const bool dataFound = (begin != string::npos && end != string::npos && begin < end);

		if (dataFound)
		{
			const size_t dataLength = end - begin - 1;
			result.push_back(str.substr(begin + 1, dataLength));
			startFrom = end + 1;
		}
		else
		{
			startFrom = string::npos;
		}
	}

	return result;
}

vector<string> SplitString(string const& str, string const& delimiter)
{
	vector<string> result;
	size_t startIndex = 0;

	while (startIndex < str.length())
	{
		const size_t delimiterIndex = str.find(delimiter, startIndex);
		const bool delimiterFound = (delimiterIndex != string::npos);
		const size_t partLength = delimiterFound ? delimiterIndex - startIndex : string::npos;

		result.push_back(str.substr(startIndex, partLength));
		startIndex = delimiterFound ? delimiterIndex + delimiter.length() : string::npos;
	}

	return result;
}

vector<set<string>> StrVectorToStrSetVector(vector<string> const& strVector)
{
	vector<set<string>> result;
	
	for (auto str : strVector)
	{
		result.push_back({ str });
	}

	return result;
}

set<string> StrVectorToStrSet(vector<string> const& strVector)
{
	set<string> result;

	for (auto str : strVector)
	{
		result.insert(str);
	}

	return result;
}

string GetInBracePart(string const& str, char lbrace, char rbrace)
{
	const vector<string> parts = GetInBraceParts(str, lbrace, rbrace);
	assert(parts.size() == 1);
	return parts[0];
}

void ParseTransition(string const& str, string &srcState, string &input, string &targState)
{
	const vector<string> parts = SplitString(str, " = ");
	assert(parts.size() == 2);

	const vector<string> leftPartData = SplitString(GetInBracePart(str, '(', ')'), ", ");
	assert(leftPartData.size() == 2);

	srcState = leftPartData[0];
	input = leftPartData[1];
	targState = parts[1];
}

NFA ReadFromFile(const char fileName[])
{
	NFA result;
	ifstream inFile(fileName);

	{
		string definition;
		getline(inFile, definition);

		const vector<string> defParts = GetInBraceParts(definition, '{', '}');
		assert(defParts.size() == 4);

		result.states = StrVectorToStrSetVector(SplitString(defParts[0], ", "));
		result.inputs = SplitString(defParts[1], ", ");
		result.startState = defParts[2];
		result.acceptStates = StrVectorToStrSet(SplitString(defParts[3], ", "));
	}

	{
		const vector<set<string>> emptyNode(result.inputs.size());
		result.table.assign(result.states.size(), emptyNode);
	}

	string curLine;
	while (getline(inFile, curLine))
	{
		string srcState, input, targState;
		ParseTransition(curLine, srcState, input, targState);
		result.InsertState({ srcState }, input, { targState });
	}

	return result;
}

void DefineAllPowersetStates(NFA &nfa)
{
	for (size_t st = 0; st < nfa.table.size(); ++st)
	{
		assert(nfa.states.size() == nfa.table.size());

		for (size_t in = 0; in < nfa.inputs.size(); ++in)
		{
			if (!nfa.table[st][in].empty() && nfa.GetStateIndex(nfa.table[st][in]) == NFA::BAD_INDEX)
			{
				nfa.AddPowersetState(nfa.table[st][in]);
			}
		}
	}
}

bool StateIsAccepting(set<string> const& state, set<string> const& acceptingStates)
{
	for (auto acceptingState : acceptingStates)
	{
		if (state.find(acceptingState) != state.cend())
		{
			return true;
		}
	}

	return false;
}

vector<bool> GetAcceptingStates(NFA const& nfa)
{
	vector<bool> result(nfa.states.size(), false);

	for (size_t st = 0; st < nfa.states.size(); ++st)
	{
		if (StateIsAccepting(nfa.states[st], nfa.acceptStates))
		{
			result[st] = true;
		}
	}

	return result;
}

string IndexToNewState(size_t index)
{
	if (index < 26)
	{
		string result;
		result.push_back('A' + index);
		return result;
	}
	else
	{
		string result("S");
		result += to_string(index - 26);
		return result;
	}
}

void PrintAsDFA(NFA const& nfa, const char fileName[])
{
	ofstream outFile(fileName);
	outFile << "M = ({";

	for (size_t st = 0; st < nfa.states.size(); ++st)
	{
		if (st != 0)
		{
			outFile << ", ";
		}
		outFile << IndexToNewState(st);
	}

	outFile << "}, {";

	for (size_t in = 0; in < nfa.inputs.size(); ++in)
	{
		if (in != 0)
		{
			outFile << ", ";
		}
		outFile << nfa.inputs[in];
	}

	outFile << "}, F, {" << IndexToNewState(nfa.GetStateIndex({ nfa.startState })) << "}, {";

	const vector<bool> acceptingStates = GetAcceptingStates(nfa);
	bool printedAlready = false;
	for (size_t st = 0; st < nfa.states.size(); ++st)
	{
		if (acceptingStates[st])
		{
			if (printedAlready)
			{
				outFile << ", ";
			}
			outFile << IndexToNewState(st);
			printedAlready = true;
		}
	}

	outFile << "})\n";

	for (size_t st = 0; st < nfa.states.size(); ++st)
	{
		for (size_t in = 0; in < nfa.inputs.size(); ++in)
		{
			if (!nfa.table[st][in].empty())
			{
				outFile << "F(" << IndexToNewState(st) << ", " << nfa.inputs[in] << ") = " <<
					IndexToNewState(nfa.GetStateIndex(nfa.table[st][in])) << "\n";
			}
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	NFA nfa = ReadFromFile("input.txt");
	DefineAllPowersetStates(nfa);
	PrintAsDFA(nfa, "output.txt");
	return 0;
}
