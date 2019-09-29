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

	CMooreData(InputSignal inputAmount, MachineState stateAmount):
		m_table(inputAmount, stateAmount, 0),
		m_output(stateAmount, 0)
	{}
};

struct StateOutputPair
{
	MachineState state;
	OutputSignal output;

	bool operator<(StateOutputPair const& right) const
	{
		if (this->state == right.state)
			return (this->output < right.output);
		else
			return (this->state < right.state);
	}

	bool operator!=(StateOutputPair const& other) const
	{
		return (state != other.state || output != other.output);
	}
};
typedef CMatrix<StateOutputPair> MealyData;

class CMachineTypedData
{
public:
	CMachineTypedData(MachineType type, InputSignal inputAmount, MachineState stateAmount):
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
		default:
			assert(false);
		}
	}

	CMachineTypedData(CMachineTypedData const& other):
		m_type(other.m_type)
	{
		switch (m_type)
		{
		case MachineType::MOORE:
			m_data = new CMooreData(*static_cast<CMooreData*>(other.m_data));
			break;
		case MachineType::MEALY:
			m_data = new MealyData(*static_cast<MealyData*>(other.m_data));
			break;
		default:
			assert(false);
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
		default:
			assert(false);
		}
	}

	MachineType GetType() const
	{
		return m_type;
	}

	CMooreData& GetMooreData()
	{
		assert(m_type == MachineType::MOORE);
		return *static_cast<CMooreData*>(m_data);
	}

	CMooreData const& GetMooreData() const
	{
		assert(m_type == MachineType::MOORE);
		return *static_cast<CMooreData*>(m_data);
	}

	MealyData& GetMealyData()
	{
		assert(m_type == MachineType::MEALY);
		return *static_cast<MealyData*>(m_data);
	}

	MealyData const& GetMealyData() const
	{
		assert(m_type == MachineType::MEALY);
		return *static_cast<MealyData*>(m_data);
	}
private:
	MachineType m_type;
	void *m_data;
};

class CMachineData
{
public:
	InputSignal m_inputAmount;
	OutputSignal m_outputAmount;
	MachineState m_stateAmount;
	CMachineTypedData m_typedData;

	CMachineData(MachineType type, InputSignal inputAmount, OutputSignal outputAmount, MachineState stateAmount):
		m_inputAmount(inputAmount),
		m_outputAmount(outputAmount),
		m_stateAmount(stateAmount),
		m_typedData(type, inputAmount, stateAmount)
	{}
};
