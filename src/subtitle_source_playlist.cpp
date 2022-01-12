#include "subtitle_source_interface.hpp"
#include "lib_modules/core/module.hpp" // KHost
#include "lib_utils/clock.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/log_sink.hpp"
#include "lib_utils/time.hpp" //timeInMsToStr
#include <cassert>
#include <cmath> //abs, lround
#include <fstream>

std::string getContentTtml(const std::vector<char> &input, int64_t referenceTimeInMs, uint64_t segmentDurationInMs, int64_t startTimeInMs, int64_t ebuttdOffsetInMs, Modules::KHost *host);

SubtitleSourceProcessorEverGrowingFile::SubtitleSourceProcessorEverGrowingFile(Modules::KHost *host, const std::string &playlistFn, uint64_t segmentDurationInMs, int64_t sleepInMs)
	: host(host), playlistFn(playlistFn), segmentDurationInMs(segmentDurationInMs), sleepInMs(sleepInMs) {
	assert(!playlistFn.empty());

	std::ifstream file(playlistFn);
	if (!file.is_open())
		host->log(Error, format("Can't open subtitle playlist file \"%s\". Start may occur with a delay.", playlistFn).c_str());

	playlistDir = playlistFn;
	while(playlistDir.back() != '\\' && playlistDir.back() != '/')
		playlistDir.pop_back();
}

ISubtitleSourceProcessor::Result SubtitleSourceProcessorEverGrowingFile::process(int64_t startTimeInMs, int &segNum) {
	std::ifstream file(playlistFn);
	if (!file.is_open()) {
		host->log(Error, format("Can't open subtitle playlist file \"%s\". Sleeping for %sms.",
		        playlistFn, sleepInMs).c_str());
		return {};
	}
	file.seekg(lastFilePos);

	std::string line;
	if (!std::getline(file, line))
		return {};

	host->log(Warning, format("Opening \"%s\"", line).c_str());

	//scan line
	std::string subtitleFn;
	int64_t timestampIn180k;
	{
		auto const MAX_PATH = 4096;
		char subtitleFnRaw[MAX_PATH];
		int hour, minute, second, ms;
		int ret = sscanf(line.c_str(), "%d:%02d:%02d.%03d,%4095s",
		        &hour, &minute, &second, &ms, subtitleFnRaw);
		if(ret != 5) {
			host->log(Error, format("Invalid arguments in line \"%s\": will retry in %sms.",
			        line, sleepInMs).c_str());
			return {};
		}

		subtitleFn = subtitleFnRaw;
		timestampIn180k = timescaleToClock((((int64_t)hour * 60 + minute) * 60 + second) * 1000 + ms, 1000);
	}
	subtitleFn = playlistDir + subtitleFn;

	//open file
	std::ifstream ifs(subtitleFn);
	if (!ifs.is_open()) {
		host->log(Error, format("Can't open subtitle media file \"%s\": will retry in %sms.",
		        subtitleFn, sleepInMs).c_str());
		return {};
	}

	//reference time: detect shifts in manifest timestamps
	auto const tolerance = 0.20;
	auto computedNumSegment = clockToTimescale(timestampIn180k, 1000) / (double)segmentDurationInMs;
	if (std::abs(computedNumSegment - segNum) > tolerance) {
		computedNumSegment = std::lround(computedNumSegment);
		host->log(Warning, format("Shifting segment number from %s to %s.", segNum, (int)computedNumSegment).c_str());
		segNum = (int)computedNumSegment;
	}
	const int64_t referenceTimeInMs = segNum * segmentDurationInMs;

	lastFilePos = lastFilePos + line.size() + 1;
	host->log(Warning, format("Manifest file position=%s, timestamp=%sms  ;  media filename=%s, media start time=%sms\n",
	        (int)lastFilePos, clockToTimescale(timestampIn180k, 1000), subtitleFn, referenceTimeInMs).c_str());

	//get size
	auto pbuf = ifs.rdbuf();
	std::size_t size = pbuf->pubseekoff(0, ifs.end, ifs.in);
	pbuf->pubseekpos(0, ifs.in);

	//get data as a buffer
	std::vector<char> input(size);
	pbuf->sgetn(input.data(), size);
	ifs.close();

	return { getContentTtml(input, referenceTimeInMs, segmentDurationInMs, startTimeInMs, 0, host), timestampIn180k };
}
