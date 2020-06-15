// MachineMinimization.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MachineData.h"

using namespace std;

void SkipChar(ifstream &stream, char chr)
{
	if (stream.get() != chr)
		assert(false);
}

MachineType ParseTypeStr(string const& str)
{
	if (str == "<MR>")
		return MachineType::MOORE;
	else if (str == "<ML>")
		return MachineType::MEALY;
	else
	{
		assert(false);
		return MachineType::MOORE;
	}
}

vector<string> SplitString(string const& str, char delimiter, size_t partCount)
{
	assert(partCount > 0);

	vector<string> result;
	result.reserve(partCount);
	size_t startIndex = 0;

	while (result.size() < partCount && startIndex < str.length())
	{
		const size_t delimiterIndex = str.find(delimiter, startIndex);
		const bool delimiterFound = (delimiterIndex != string::npos);
		const size_t partLength = delimiterFound ? delimiterIndex - startIndex : string::npos;

		result.push_back(str.substr(startIndex, partLength));
		startIndex = delimiterFound ? delimiterIndex + 1 : string::npos;
	}

	return ((result.size() == partCount && startIndex == string::npos)
		? result
		: vector<string>());
}

vector<string> GetRowElements(ifstream &stream, MachineState stateAmount)
{
	string rowStr;
	getline(stream, rowStr);

	vector<string> strParts = SplitString(rowStr, ',', stateAmount);
	assert(!strParts.empty());

	return strParts;
}

size_t GetNodeNumber(string const& str)
{
	assert(str.length() > 1);
	return stoi(str.substr(1));
}

unique_ptr<CMachineData> ReadFromFile(const char fileName[])
{
	ifstream file(fileName);
	assert((file.rdstate() & ifstream::failbit) == 0);

	InputSignal inputAmount;
	file >> inputAmount;
	SkipChar(file, ',');

	OutputSignal outputAmount;
	file >> outputAmount;
	SkipChar(file, ',');

	MachineState stateAmount;
	file >> stateAmount;
	SkipChar(file, '\n');

	string typeStr;
	getline(file, typeStr);
	const MachineType type = ParseTypeStr(typeStr);

	unique_ptr<CMachineData> result(new CMachineData(type, inputAmount, outputAmount, stateAmount));

	switch (type)
	{
	case MachineType::MOORE:
	{
		CMooreData &moore = result->m_typedData.GetMooreData();

		const vector<string> outputStrings = GetRowElements(file, stateAmount);
		for (MachineState st = 0; st < stateAmount; ++st)
		{
			moore.m_output[st] = GetNodeNumber(outputStrings[st]);
		}

		for (InputSignal curInput = 0; curInput < inputAmount; ++curInput)
		{
			const vector<string> transitionStrings = GetRowElements(file, stateAmount);

			for (MachineState curState = 0; curState < stateAmount; ++curState)
			{
				moore.m_table(curInput, curState) = GetNodeNumber(transitionStrings[curState]);
			}
		}

		break;
	}

	case MachineType::MEALY:
	{
		MealyData &mealy = result->m_typedData.GetMealyData();

		for (InputSignal curInput = 0; curInput < inputAmount; ++curInput)
		{
			const vector<string> transitionStrings = GetRowElements(file, stateAmount);

			for (MachineState curState = 0; curState < stateAmount; ++curState)
			{
				const vector<string> components = SplitString(transitionStrings[curState], ' ', 2);
				assert(components.size() == 2);

				mealy(curInput, curState).state = GetNodeNumber(components[0]);
				mealy(curInput, curState).output = GetNodeNumber(components[1]);
			}
		}

		break;
	}

	default:
		assert(false);
	}

	return result;
}

void WriteToFile(const char fileName[], CMachineData const& data)
{
	ofstream file(fileName);
	file << data.m_inputAmount << "," << data.m_outputAmount << "," << data.m_stateAmount << "\n";

	switch (data.m_typedData.GetType())
	{
	case MachineType::MOORE:
	{
		file << "<MR>\n";
		CMooreData const& mooreData = data.m_typedData.GetMooreData();

		for (vector<OutputSignal>::const_iterator it = mooreData.m_output.cbegin(); it != mooreData.m_output.cend(); ++it)
		{
			file << "Y" << *it;
			if (it + 1 != mooreData.m_output.cend())
				file << ",";
		}
		file << "\n";

		for (InputSignal in = 0; in < data.m_inputAmount; ++in)
		{
			for (MachineState st = 0; st < data.m_stateAmount; ++st)
			{
				file << "Q" << mooreData.m_table(in, st);
				if (st + 1 < data.m_stateAmount)
					file << ",";
			}
			file << "\n";
		}

		break;
	}

	case MachineType::MEALY:
	{
		file << "<ML>\n";
		MealyData const& mealyData = data.m_typedData.GetMealyData();

		for (InputSignal in = 0; in < data.m_inputAmount; ++in)
		{
			for (MachineState st = 0; st < data.m_stateAmount; ++st)
			{
				file << "S" << mealyData(in, st).state << " " << "Y" << mealyData(in, st).output;
				if (st + 1 < data.m_stateAmount)
					file << ",";
			}
			file << "\n";
		}

		break;
	}

	default:
		assert(false);
	}
}

bool TransitionExist(CMachineData const& machine, MachineState src, MachineState dest)
{
	for (InputSignal in = 0; in < machine.m_inputAmount; ++in)
	{
		switch (machine.m_typedData.GetType())
		{
		case MachineType::MOORE:
		{
			CMooreData const& mooreData = machine.m_typedData.GetMooreData();
			if (mooreData.m_table(in, src) == dest)
			{
				return true;
			}
			break;
		}

		case MachineType::MEALY:
		{
			MealyData const& mealyData = machine.m_typedData.GetMealyData();
			if (mealyData(in, src).state == dest)
			{
				return true;
			}
			break;
		}

		default:
			assert(false);
			return false;
		}
	}

	return false;
}

vector<bool> GetReachableStates(CMachineData const& machine)
{
	vector<bool> reachableStates(machine.m_stateAmount, false);
	reachableStates[0] = true;

	bool foundNewStates;
	do
	{
		foundNewStates = false;

		for (MachineState src = 0; src < machine.m_stateAmount && !foundNewStates; ++src)
		{
			if (reachableStates[src])
			{
				for (MachineState dest = 0; dest < machine.m_stateAmount && !foundNewStates; ++dest)
				{
					if (!reachableStates[dest] && TransitionExist(machine, src, dest))
					{
						foundNewStates = true;
						reachableStates[dest] = true;
					}
				}
			}
		}
	}
	while (foundNewStates);

	return reachableStates;
}

size_t CountTrueValues(vector<bool> const& data)
{
	size_t cnt = 0;

	for (bool elem : data)
	{
		if (elem)
		{
			++cnt;
		}
	}

	return cnt;
}

unique_ptr<CMachineData> SaveSpecifiedStates(CMachineData const& machine, vector<bool> const& states)
{
	const MachineState stateCount = CountTrueValues(states);

	vector<MachineState> newStateFunction(machine.m_stateAmount, machine.m_stateAmount);
	MachineState newState = 0;
	for (MachineState oldState = 0; oldState < machine.m_stateAmount; ++oldState)
	{
		if (states[oldState])
		{
			newStateFunction[oldState] = newState;
			++newState;
		}
	}

	unique_ptr<CMachineData> result(new CMachineData(machine.m_typedData.GetType(),
		machine.m_inputAmount, machine.m_outputAmount, stateCount));

	switch (machine.m_typedData.GetType())
	{
	case MachineType::MOORE:
	{
		CMooreData const& oldMooreData = machine.m_typedData.GetMooreData();
		CMooreData &newMooreData = result->m_typedData.GetMooreData();

		MachineState newState = 0;
		for (MachineState oldState = 0; oldState < machine.m_stateAmount; ++oldState)
		{
			if (states[oldState])
			{
				newMooreData.m_output[newState] = oldMooreData.m_output[oldState];

				for (InputSignal in = 0; in < machine.m_inputAmount; ++in)
				{
					newMooreData.m_table(in, newState) = newStateFunction[oldMooreData.m_table(in, oldState)];
				}

				++newState;
			}
		}

		break;
	}

	case MachineType::MEALY:
	{
		MealyData const& oldMealyData = machine.m_typedData.GetMealyData();
		MealyData &newMealyData = result->m_typedData.GetMealyData();

		MachineState newState = 0;
		for (MachineState oldState = 0; oldState < machine.m_stateAmount; ++oldState)
		{
			if (states[oldState])
			{
				for (InputSignal in = 0; in < machine.m_inputAmount; ++in)
				{
					newMealyData(in, newState).output = oldMealyData(in, oldState).output;
					newMealyData(in, newState).state = newStateFunction[oldMealyData(in, oldState).state];
				}

				++newState;
			}
		}

		break;
	}

	default:
		assert(false);
	}

	return result;
}

unique_ptr<CMachineData> RemoveUnreachableStates(CMachineData const& machine)
{
	return SaveSpecifiedStates(machine, GetReachableStates(machine));
}

bool StatesAreEquivalent(CMachineData const& machine, MachineState a, MachineState b)
{
	switch (machine.m_typedData.GetType())
	{
	case MachineType::MOORE:
	{
		CMooreData const& mooreData = machine.m_typedData.GetMooreData();

		if (mooreData.m_output[a] != mooreData.m_output[b])
		{
			return false;
		}

		for (InputSignal in = 0; in < machine.m_inputAmount; ++in)
		{
			if (mooreData.m_table(in, a) != mooreData.m_table(in, b))
			{
				return false;
			}
		}

		return true;
	}

	case MachineType::MEALY:
	{
		MealyData const& mealyData = machine.m_typedData.GetMealyData();

		for (InputSignal in = 0; in < machine.m_inputAmount; ++in)
		{
			if (mealyData(in, a) != mealyData(in, b))
			{
				return false;
			}
		}

		return true;
	}

	default:
		assert(false);
		return false;
	}
}

void ReplaceStates(CMachineData &machine, MachineState find, MachineState replace)
{
	for (MachineState st = 0; st < machine.m_stateAmount; ++st)
	{
		for (InputSignal in = 0; in < machine.m_inputAmount; ++in)
		{
			switch (machine.m_typedData.GetType())
			{
			case MachineType::MOORE:
			{
				CMooreData &mooreData = machine.m_typedData.GetMooreData();

				if (mooreData.m_table(in, st) == find)
				{
					mooreData.m_table(in, st) = replace;
				}

				break;
			}

			case MachineType::MEALY:
			{
				MealyData &mealyData = machine.m_typedData.GetMealyData();

				if (mealyData(in, st).state == find)
				{
					mealyData(in, st).state = replace;
				}

				break;
			}

			default:
				assert(false);
			}
		}
	}
}

unique_ptr<CMachineData> MergeEquivalentStates(CMachineData machine)
{
	vector<bool> activeStates(machine.m_stateAmount, true);

	bool changesMade;
	do
	{
		changesMade = false;

		for (MachineState a = 0; a < machine.m_stateAmount && !changesMade; ++a)
		{
			if (activeStates[a])
			{
				for (MachineState b = a + 1; b < machine.m_stateAmount && !changesMade; ++b)
				{
					if (activeStates[b] && StatesAreEquivalent(machine, a, b))
					{
						changesMade = true;
						activeStates[b] = false;
						ReplaceStates(machine, b, a);
					}
				}
			}
		}
	}
	while (changesMade);

	return SaveSpecifiedStates(machine, activeStates);
}

unique_ptr<CMachineData> MinimizeMachine(CMachineData const& machine)
{
	return MergeEquivalentStates(*RemoveUnreachableStates(machine));
}

int _tmain(int argc, _TCHAR* argv[])
{
	WriteToFile("output.txt", *MinimizeMachine(*ReadFromFile("input.txt")));
	return 0;
}
