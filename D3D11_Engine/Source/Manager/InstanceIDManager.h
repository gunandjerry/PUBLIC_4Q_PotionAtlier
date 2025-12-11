#pragma once
#include <iostream>
#include <set>
#include <deque>
#include <Core/TSingleton.h>
#include <mutex>

class InstanceIDManager;
extern InstanceIDManager& instanceIDManager;

class InstanceIDManager : public TSingleton<InstanceIDManager>
{
    template <typename T>
    friend class TSingleton;
    InstanceIDManager();
    virtual ~InstanceIDManager() override;
private:
    std::set<unsigned int> activeIDs;            // 사용 중인 ID를 추적
    std::deque<unsigned int> availableIDs;       // 재사용 가능한 ID 목록
    unsigned int nextID = 0;
    std::mutex mutex;
public:
    unsigned int getUniqueID();
    void returnID(unsigned int id);
    /*반납한 ID 대기열을 정렬합니다.*/
    void SortReturnID();
};
