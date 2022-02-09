#pragma once

#include <string>

struct ISubtitleSourceProcessor {
	struct Result {
		std::string text;
		int64_t timestampIn180k = -1;
	};

	virtual ~ISubtitleSourceProcessor() = default;
	virtual Result process(int64_t startTimeInMs, int &segNum) = 0;
};

namespace Modules {
struct KHost;
}

struct SubtitleSourceProcessorEverGrowingFile : ISubtitleSourceProcessor {
		SubtitleSourceProcessorEverGrowingFile(Modules::KHost *host, bool ttml, const std::string &playlistFn, uint64_t segmentDurationInMs, int64_t sleepInMs);
		Result process(int64_t startTimeInMs, int &segNum) final;

	private:
		Modules::KHost *host;
		const bool ttml;
		const std::string playlistFn;
		const uint64_t segmentDurationInMs;
		const int64_t sleepInMs;
		int64_t ttmlMediaOffsetInMs = 0;
		int lastFilePos = 0;
		std::string playlistDir;
};

struct SubtitleSourceProcessorSyntheticTtml : ISubtitleSourceProcessor {
		SubtitleSourceProcessorSyntheticTtml(uint64_t segmentDurationInMs);
		Result process(int64_t startTimeInMs, int &segNum) final;

		static Result generate(int64_t startTimeInMs, int segNum, int64_t segmentDurationInMs, bool empty);

	private:
		const uint64_t segmentDurationInMs;
};

struct SubtitleSourceProcessorSyntheticWebvtt : ISubtitleSourceProcessor {
		SubtitleSourceProcessorSyntheticWebvtt(uint64_t segmentDurationInMs);
		Result process(int64_t startTimeInMs, int &segNum) final;

		static Result generate(int64_t startTimeInMs, int segNum, int64_t segmentDurationInMs, bool empty);

	private:
		const uint64_t segmentDurationInMs;
};
