// MachineConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MachineData.h"

void SkipChar(std::ifstream &stream, char chr)
{
	if (stream.get() != chr)
		assert(false);
}

MachineType ParseTypeStr(std::string const& str)
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

std::unique_ptr<std::vector<std::string>> SplitString(std::string const& str, char delimiter, size_t reserveCount)
{
	std::unique_ptr<std::vector<std::string>> result(new std::vector<std::string>);
	result->reserve(reserveCount);

	size_t startFrom = 0;
	size_t findResult;

	while (startFrom < str.length() && ((findResult = str.find(delimiter, startFrom)) != std::string::npos))
	{
		assert(findResult >= startFrom);
		const size_t curLength = (findResult - startFrom);
		result->push_back(str.substr(startFrom, curLength));
		startFrom = findResult + 1;
	}

	result->push_back((startFrom < str.length()) ? str.substr(startFrom) : "");
	return result;
}

std::unique_ptr<std::vector<std::string>> GetRowElements(std::ifstream &stream, MachineState stateAmount)
{
	std::string rowStr;
	std::getline(stream, rowStr);

	std::unique_ptr<std::vector<std::string>> strParts = SplitString(rowStr, ',', stateAmount);
	assert(strParts->size() == stateAmount);

	return strParts;
}

size_t GetNodeNumber(std::string const& str)
{
	assert(str.length() > 1);
	return std::stoi(str.substr(1));
}

std::unique_ptr<CMachineData> ReadFromFile(const char fileName[])
{
	std::ifstream file(fileName);
	assert((file.rdstate() & std::ifstream::failbit) == 0);

	InputSignal inputAmount;
	file >> inputAmount;
	SkipChar(file, ',');

	OutputSignal outputAmount;
	file >> outputAmount;
	SkipChar(file, ',');

	MachineState stateAmount;
	file >> stateAmount;
	SkipChar(file, '\n');

	std::string typeStr;
	std::getline(file, typeStr);
	const MachineType type = ParseTypeStr(typeStr);

	std::unique_ptr<CMachineData> result(new CMachineData(type, inputAmount, outputAmount, stateAmount));

	switch (type)
	{
	case MachineType::MOORE:
	{
		CMooreData &moore = result->m_typedData.GetMooreData();

		const std::unique_ptr<std::vector<std::string>> outputStrings = GetRowElements(file, stateAmount);
		for (MachineState st = 0; st < stateAmount; ++st)
		{
			moore.m_output[st] = GetNodeNumber((*outputStrings)[st]);
		}

		for (InputSignal curInput = 0; curInput < inputAmount; ++curInput)
		{
			const std::unique_ptr<std::vector<std::string>> transitionStrings = GetRowElements(file, stateAmount);

			for (size_t curState = 0; curState < stateAmount; ++curState)
			{
				moore.m_table(curInput, curState) = GetNodeNumber((*transitionStrings)[curState]);
			}
		}

		break;
	}

	case MachineType::MEALY:
	{
		MealyData &mealy = result->m_typedData.GetMealyData();

		for (InputSignal curInput = 0; curInput < inputAmount; ++curInput)
		{
			const std::unique_ptr<std::vector<std::string>> transitionStrings = GetRowElements(file, stateAmount);

			for (size_t curState = 0; curState < stateAmount; ++curState)
			{
				const std::unique_ptr<std::vector<std::string>> components = SplitString((*transitionStrings)[curState], ' ', 2);
				assert(components->size() == 2);

				mealy(curInput, curState).state = GetNodeNumber((*components)[0]);
				mealy(curInput, curState).output = GetNodeNumber((*components)[1]);
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
	std::ofstream file(fileName);
	file << data.m_inputAmount << "," << data.m_outputAmount << "," << data.m_stateAmount << "\n";

	switch (data.m_typedData.GetType())
	{
	case MachineType::MOORE:
	{
		file << "<MR>\n";
		CMooreData const& mooreData = data.m_typedData.GetMooreData();

		for (std::vector<OutputSignal>::const_iterator it = mooreData.m_output.cbegin(); it != mooreData.m_output.cend(); ++it)
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

std::unique_ptr<CMachineData> MealyToMoore(CMachineData const& mealy)
{
	MealyData const& mealyData = mealy.m_typedData.GetMealyData();

	typedef std::map<StateOutputPair, MachineState> NewStateFunction;
	NewStateFunction newStateFunction;

	for (InputSignal in = 0; in < mealy.m_inputAmount; ++in)
	{
		for (MachineState st = 0; st < mealy.m_stateAmount; ++st)
		{
			newStateFunction.insert(NewStateFunction::value_type(mealyData(in, st), 0));
		}
	}

	{
		MachineState curState = 0;
		for (NewStateFunction::iterator it = newStateFunction.begin(); it != newStateFunction.end(); ++it)
		{
			it->second = curState;
			++curState;
		}
	}

	std::unique_ptr<CMachineData> moore(new CMachineData(
		MachineType::MOORE, mealy.m_inputAmount, mealy.m_outputAmount, newStateFunction.size()));
	CMooreData &mooreData = moore->m_typedData.GetMooreData();

	{
		MachineState targState = 0;
		for (NewStateFunction::const_iterator it = newStateFunction.cbegin(); it != newStateFunction.cend(); ++it)
		{
			const MachineState srcState = it->first.state;

			for (InputSignal curInput = 0; curInput < moore->m_inputAmount; ++curInput)
			{
				mooreData.m_table(curInput, targState) = newStateFunction[mealyData(curInput, srcState)];
			}

			mooreData.m_output[targState] = it->first.output;
			++targState;
		}
	}

	return moore;
}

std::unique_ptr<CMachineData> MooreToMealy(CMachineData const& moore)
{
	CMooreData const& mooreData = moore.m_typedData.GetMooreData();

	std::unique_ptr<CMachineData> mealy(new CMachineData(
		MachineType::MEALY, moore.m_inputAmount, moore.m_outputAmount, moore.m_stateAmount));
	MealyData &mealyData = mealy->m_typedData.GetMealyData();

	for (InputSignal in = 0; in < moore.m_inputAmount; ++in)
	{
		for (MachineState st = 0; st < moore.m_stateAmount; ++st)
		{
			mealyData(in, st).state = mooreData.m_table(in, st);
			mealyData(in, st).output = mooreData.m_output[mealyData(in, st).state];
		}
	}

	return mealy;
}

int _tmain(int argc, _TCHAR* argv[])
{
	const std::unique_ptr<CMachineData> inputMachine = ReadFromFile("input.txt");

	std::unique_ptr<CMachineData> outputMachine;
	switch (inputMachine->m_typedData.GetType())
	{
	case MachineType::MOORE:
		outputMachine = MooreToMealy(*inputMachine);
		break;

	case MachineType::MEALY:
		outputMachine = MealyToMoore(*inputMachine);
		break;

	default:
		assert(false);
	}

	WriteToFile("output.txt", *outputMachine);
	return 0;
}
