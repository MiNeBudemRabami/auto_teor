#pragma once

#include "Matrix.h"

typedef size_t InputSignal;
typedef size_t OutputSignal;
typedef size_t MachineState;
enum class MachineType { MOORE, MEALY };

class CMooreData
{
public:
	CMatrix<MachineState> m_table;
	std::vector<OutputSignal> m_output;

	CMooreData(InputSignal inputAmount, MachineState stateAmount) :
		m_table(inputAmount, stateAmount, 0),
		m_output(stateAmount, 0)
	{}
};

struct StateOutputPair
{
	MachineState state;
	OutputSignal output;

	bool operator <(StateOutputPair const& right) const
	{
		if (this->state == right.state)
			return (this->output < right.output);
		else
			return (this->state < right.state);
	}
};
typedef CMatrix<StateOutputPair> MealyData;

class CMachineTypedData
{
public:
	CMachineTypedData(MachineType type, InputSignal inputAmount, MachineState stateAmount) :
		m_type(type)
	{
		switch (type)
		{
		case MachineType::MOORE:
			m_data = new CMooreData(inputAmount, stateAmount);
			break;
		case MachineType::MEALY:
			m_data = new MealyData(inputAmount, stateAmount, { 0, 0 });
			break;
		}
	}

	~CMachineTypedData()
	{
		switch (m_type)
		{
		case MachineType::MOORE:
			delete static_cast<CMooreData*>(m_data);
			break;
		case MachineType::MEALY:
			delete static_cast<MealyData*>(m_data);
			break;
		}
	}

	MachineType GetType() const
	{
		return m_type;
	}

	CMooreData& GetMooreData()
	{
		return *static_cast<CMooreData*>(m_data);
	}

	CMooreData const& GetMooreData() const
	{
		return *static_cast<CMooreData*>(m_data);
	}

	MealyData& GetMealyData()
	{
		return *static_cast<MealyData*>(m_data);
	}

	MealyData const& GetMealyData() const
	{
		return *static_cast<MealyData*>(m_data);
	}
private:
	MachineType m_type;
	void* m_data;
};

class CMachineData
{
public:
	InputSignal m_inputAmount;
	OutputSignal m_outputAmount;
	MachineState m_stateAmount;
	CMachineTypedData m_typedData;

	CMachineData(MachineType type, InputSignal inputAmount, OutputSignal outputAmount, MachineState stateAmount) :
		m_inputAmount(inputAmount),
		m_outputAmount(outputAmount),
		m_stateAmount(stateAmount),
		m_typedData(type, inputAmount, stateAmount)
	{}
};
