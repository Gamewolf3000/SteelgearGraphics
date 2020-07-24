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

// Beh�vs nog tv� vektorer vid updatering
	// En som h�ller koll p� alla som updaterats sedan senaste render callet
		// F�r alla i den, n�r render kallas, byt updateringsbuffer
		// Alla i denna l�ggs sedan till i den andra vektorn och denna clearas
	// En som h�ller koll p� alla som updaterats sedan senaste faktiska renderingen
		// F�r alla i den, byt den aktiva buffern och cleara sedan

// �ndra i handlerbasklassen/subklasserna s� att liknande system finns d�r f�r entityData samt groupData (l�gga till funktion i basklassen f�r just sj�lva updateringen �r nog enklast s� kan den hantera vektorerna och s�nt)
// G�r om UpdateBuffers. On�digt att updatera gpu sidan om den inte anv�nds. Ist�llet, varje g�ng en buffer ska anv�ndas s� kolla om den �r updaterad, och i s� fall updatera gpun
// Forts�tt p� HandleRenderJob