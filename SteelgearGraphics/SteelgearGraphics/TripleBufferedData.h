#pragma once

#include <utility>

template<class T>
class TripleBufferedData
{
private:
	short currentlyActive = 0;
	short lastUpdated = 0;
	short nextToUpdate = 1;
	bool updatedInternal = false;
	bool updatedReturn = false;
	T storedData[3];

public:
	TripleBufferedData() = default;
	TripleBufferedData(const T& t);
	TripleBufferedData(const T& t1, const T& t2, const T& t3);
	TripleBufferedData(T&& t1, T&& t2, T&& t3);
	const TripleBufferedData<T>& operator=(const T& data);
	const TripleBufferedData<T>& operator=(TripleBufferedData<T>&& other);

	bool Updated();
	void MarkAsNotUpdated();
	void MarkAsUpdated();
	T& GetActive();
	T& GetToUpdate();
	void UpdateData(const T& data);
	void UpdateData(T&& data);
	void SwitchActiveBuffer();
	void SwitchUpdateBuffer();

};

template<class T>
inline TripleBufferedData<T>::TripleBufferedData(const T & t) : storedData{ t, T(), T()}
{
	// EMPTY
}

template<class T>
inline TripleBufferedData<T>::TripleBufferedData(const T& t1, const T& t2, const T& t3) : storedData{t1, t2, t3}
{
	// EMPTY
}

template<class T>
inline TripleBufferedData<T>::TripleBufferedData(T && t1, T && t2, T && t3) : storedData{ std::move(t1), std::move(t2), std::move(t3) }
{
	//EMPTY
}

template<class T>
inline const TripleBufferedData<T>& TripleBufferedData<T>::operator=(const T & data)
{
	storedData[currentlyActive] = data;
	return *this;
}

template<class T>
inline const TripleBufferedData<T>& TripleBufferedData<T>::operator=(TripleBufferedData && other)
{
	this->currentlyActive = other.currentlyActive;
	this->lastUpdated = other.lastUpdated;
	this->nextToUpdate = other.nextToUpdate;
	this->updatedInternal = other.updatedInternal;
	this->updatedReturn = other.updatedReturn;

	for(int i = 0; i < 3; ++i)
		this->storedData[i] = std::move(other.storedData[i]);

	return *this;
}

template<class T>
inline bool TripleBufferedData<T>::Updated()
{
	return updatedReturn;
}

template<class T>
inline void TripleBufferedData<T>::MarkAsNotUpdated()
{
	// If an update has been made after last switch we want to consider us updated externally as well so that an update is not missed
	updatedReturn = updatedInternal;
	updatedInternal = false;
}

template<class T>
inline void TripleBufferedData<T>::MarkAsUpdated()
{
	updatedInternal = true;
}

template<class T>
inline T & TripleBufferedData<T>::GetActive()
{
	return storedData[currentlyActive];
}

template<class T>
inline T & TripleBufferedData<T>::GetToUpdate()
{
	return storedData[nextToUpdate];
}

template<class T>
inline void TripleBufferedData<T>::UpdateData(const T & data)
{
	storedData[nextToUpdate] = data;
	updatedInternal = true;
}

template<class T>
inline void TripleBufferedData<T>::UpdateData(T && data)
{
	storedData[nextToUpdate] = data;
	updatedInternal = true;
}

template<class T>
inline void TripleBufferedData<T>::SwitchActiveBuffer()
{
	if (currentlyActive != lastUpdated)
	{
		currentlyActive = lastUpdated;
		updatedReturn = true;
	}
}

template<class T>
inline void TripleBufferedData<T>::SwitchUpdateBuffer()
{
	if (!updatedInternal)
		return;

	lastUpdated = nextToUpdate;
	nextToUpdate = 3 - currentlyActive - nextToUpdate;

	// Logic behind above equation is derived from the following cases
	/*
	switch (currentlyActive)
	{
	case 0:
		switch (nextToUpdate)
		{
		case 1:
			nextToUpdate = 2;
			break;
		case 2:
			nextToUpdate = 1;
			break;
		}
		break;
	case 1:
		switch (nextToUpdate)
		{
		case 0:
			nextToUpdate = 2;
			break;
		case 2:
			nextToUpdate = 0;
			break;
		}
		break;
	case 2:
		switch (nextToUpdate)
		{
		case 0:
			nextToUpdate = 1;
			break;
		case 1:
			nextToUpdate = 0;
			break;
		}
		break;
	}
	*/
}

// Behövs nog två vektorer vid updatering
	// En som håller koll på alla som updaterats sedan senaste render callet
		// För alla i den, när render kallas, byt updateringsbuffer
		// Alla i denna läggs sedan till i den andra vektorn och denna clearas
	// En som håller koll på alla som updaterats sedan senaste faktiska renderingen
		// För alla i den, byt den aktiva buffern och cleara sedan

// Ändra i handlerbasklassen/subklasserna så att liknande system finns där för entityData samt groupData (lägga till funktion i basklassen för just själva updateringen är nog enklast så kan den hantera vektorerna och sånt)
// Gör om UpdateBuffers. Onödigt att updatera gpu sidan om den inte används. Istället, varje gång en buffer ska användas så kolla om den är updaterad, och i så fall updatera gpun
// Fortsätt på HandleRenderJob