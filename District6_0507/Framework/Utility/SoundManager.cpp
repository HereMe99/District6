#include "Framework.h"
#include "SoundManager.h"

SoundManager* SoundManager::instance = nullptr;

SoundManager::SoundManager()
{
	dsp = nullptr;
	System_Create(&soundSystem);
	soundSystem->init(SOUNDCOUNT, FMOD_INIT_NORMAL, nullptr);
}

SoundManager::~SoundManager()
{
	for (auto sound : totalSound)
	{
		delete sound.second;
	}
	
	soundSystem->release();
}

SoundManager* SoundManager::Get()
{
	return instance;
}

void SoundManager::Create()
{
	instance = NEW SoundManager();
}

void SoundManager::Release()
{
	delete instance;
}

void SoundManager::Update()
{
	soundSystem->update();
}

void SoundManager::Add(string key, string fileName, bool bgm)
{
	if(totalSound.find(key) != totalSound.end())
	{
		return;
	}
	
	SoundInfo* info = NEW SoundInfo();
	
	if (bgm)
	{
		soundSystem->createStream(fileName.c_str(), FMOD_LOOP_NORMAL,
			nullptr, &info->sound);
	}
	else
	{
		soundSystem->createSound(fileName.c_str(), FMOD_DEFAULT,
			nullptr, &info->sound);
	}

	totalSound.insert({ key, info });
}

void SoundManager::Play(string key, float volume)
{
	if (totalSound.count(key) == 0)
		return;

	soundSystem->playSound(FMOD_CHANNEL_FREE,
		totalSound[key]->sound, false, &totalSound[key]->channel);
	totalSound[key]->channel->setVolume(volume);
}

void SoundManager::Stop(string key)
{
	if (totalSound.count(key) == 0)
		return;

	totalSound[key]->channel->stop();
}

void SoundManager::Pause(string key)
{
	if (totalSound.count(key) == 0)
		return;

	totalSound[key]->channel->setPaused(true);
}

void SoundManager::Resume(string key)
{
	if (totalSound.count(key) == 0)
		return;

	totalSound[key]->channel->setPaused(false);
}

void SoundManager::SetSoundFrequency(string key, float frequency)
{
	totalSound[key]->channel->setFrequency(frequency);
}

bool SoundManager::IsPlaying(string key)
{
	bool isPlay = false;

	totalSound[key]->channel->isPlaying(&isPlay);

	return isPlay;
}

bool SoundManager::IsPaused(string key)
{
	bool isPause = false;

	totalSound[key]->channel->getPaused(&isPause);

	return isPause;
}

void SoundManager::DspHighpassSetting(string key)
{
	if (dsp == NULL)
	{
		soundSystem->createDSPByType(FMOD_DSP_TYPE_HIGHPASS, &dsp);
		dsp->setActive(true);
		dsp->setBypass(false);
		totalSound[key]->channel->addDSP(dsp, NULL);
	}
}

void SoundManager::DspSetHighpass(float amount)
{
	dsp->setParameter(FMOD_DSP_HIGHPASS_CUTOFF, amount);
}
