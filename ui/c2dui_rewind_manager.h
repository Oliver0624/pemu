//
// Rewind Manager - lightweight, state-only mode
//

#ifndef C2DUI_REWIND_MANAGER_H
#define C2DUI_REWIND_MANAGER_H

#include <deque>
#include <string>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <functional>
#include <cstdio>
#include "c2dui_text_def.h"

namespace c2dui {

    class RewindManager {
    public:
        struct RewindEntry {
            long long frameOffset = 0;
            int secondsAgo = 0;
            std::vector<uint8_t> stateData;
        };

        RewindManager() = default;

        void init(int targetFps, size_t maxMemoryMB = 128) {
            this->targetFps = targetFps;
            this->maxMemoryBytes = maxMemoryMB * 1024 * 1024;
            sampleFrames = targetFps * 2;
            frameCounter = 0;
            selection = -1;
            timelineVisible = false;
            comboLatch = false;
            totalFramesCaptured = 0;
            timingsHistory.reserve(8);
        }

        void reset() {
            entries.clear();
            frameCounter = 0;
            selection = -1;
            timelineVisible = false;
            comboLatch = false;
            selectionChanged = false;
            openStateValid = false;
            openStateData.clear();
            currentMemoryBytes = 0;
            totalFramesCaptured = 0;
            timingsHistory.clear();
        }

        // Called every frame during emulation
        void tickAndCapture(
                const std::function<bool(std::vector<uint8_t> &)> &serializeFunc) {
            frameCounter++;
            totalFramesCaptured++;
            if (frameCounter < sampleFrames) {
                return;
            }
            captureSnapshot(serializeFunc);
            frameCounter = 0;
        }

        // Handle input for rewind mode
        // Returns true if input was consumed
        bool handleInput(
                bool lbPressed, bool rbPressed,
                bool leftPressed, bool rightPressed,
                bool aPressed, bool bPressed,
                const std::function<bool(std::vector<uint8_t> &)> &serializeFunc,
                const std::function<bool(const std::vector<uint8_t> &)> &deserializeFunc,
                const std::function<void()> &clearInputFunc,
                const std::function<void()> &pauseFunc,
                const std::function<void()> &resumeFunc,
                const std::function<void()> &previewFunc,
                const std::function<void()> &clearAudioFunc,
                bool (*updateHintText)(void *userData, const char *text),
                void *userData) {

            if (timelineVisible) {
                bool left = leftPressed;
                bool right = rightPressed;
                bool changed = false;

                // Simple auto-repeat for navigation
                auto now = std::chrono::steady_clock::now();
                if (left || right) {
                    bool firstPress = (left && !(lastButtons & 1)) || (right && !(lastButtons & 2));
                    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInputTime).count();
                    
                    if (firstPress || (elapsedMs > 300 && elapsedMs > (300 + inputRepeatCount * 100))) {
                        if (left && selection > 0) {
                            selection--;
                            changed = true;
                        } else if (right && selection + 1 < (int) entries.size()) {
                            selection++;
                            changed = true;
                        }
                        if (firstPress) {
                            lastInputTime = now;
                            inputRepeatCount = 0;
                        } else {
                            inputRepeatCount++;
                        }
                    }
                } else {
                    lastInputTime = now;
                    inputRepeatCount = 0;
                }
                
                // Track buttons (1=Left, 2=Right)
                lastButtons = (left ? 1 : 0) | (right ? 2 : 0);

                if (changed) {
                    selectionChanged = true;
                    previewSelectedEntryTransactional(deserializeFunc, clearInputFunc, previewFunc, clearAudioFunc);
                    if (updateHintText) {
                        updateHintText(userData, getHintText().c_str());
                    }
                    return true;
                }

                // A confirms rewind: load state and truncate future timeline
                if (aPressed) {
                    if (selectionChanged && selection >= 0 && selection < (int) entries.size()) {
                        if (deserializeFunc(entries[selection].stateData)) {
                            // Truncate timeline after selection
                            while ((int) entries.size() > selection + 1) {
                                currentMemoryBytes -= entries.back().stateData.size();
                                entries.pop_back();
                            }
                            // Total frames should reset to the selected point's offset
                            totalFramesCaptured = entries.back().frameOffset;
                        }
                    }
                    timelineVisible = false;
                    selectionChanged = false;
                    openStateValid = false;
                    openStateData.clear();
                    resumeFunc();
                    if (clearAudioFunc) clearAudioFunc();
                    clearInputFunc();
                    return true;
                }

                // B cancels rewind: restore original (latest) state
                if (bPressed) {
                    if (openStateValid) {
                        deserializeFunc(openStateData);
                    }
                    timelineVisible = false;
                    selectionChanged = false;
                    openStateValid = false;
                    openStateData.clear();
                    resumeFunc();
                    if (clearAudioFunc) clearAudioFunc();
                    clearInputFunc();
                    return true;
                }
                return true;
            }

            // Not in rewind mode: LB+RB to open
            bool rewindCombo = lbPressed && rbPressed;
            if (rewindCombo && !comboLatch) {
                comboLatch = true;
                return openRewindTimeline(serializeFunc, deserializeFunc, clearInputFunc, pauseFunc, resumeFunc, previewFunc, clearAudioFunc, updateHintText, userData);
            } else if (!rewindCombo) {
                comboLatch = false;
            }
            return false;
        }

        // Accessors for UI
        int getSelection() const { return selection; }
        bool isTimelineVisible() const { return timelineVisible; }
        int getTargetFps() const { return targetFps; }

        void recordTiming(double ms) {
            timingsHistory.push_back(ms);
            if (timingsHistory.size() > 8) {
                timingsHistory.erase(timingsHistory.begin());
            }
            double avg = 0;
            for (double t : timingsHistory) avg += t;
            avg /= timingsHistory.size();
            adjustFrequency(avg);
        }

        void adjustFrequency(double avgSerializeMs) {
            double slowThreshold = 15.0;
            if (avgSerializeMs > slowThreshold) {
                sampleFrames = std::min(targetFps * 10, sampleFrames + 10);
            } else if (avgSerializeMs < slowThreshold / 2) {
                sampleFrames = std::max(targetFps / 4, sampleFrames - 10);
            }
        }

        // Get hint text for display
        std::string getHintText() const {
            char buf[128];
            if (entries.empty()) {
                std::snprintf(buf, sizeof(buf), "%s | %s", TEXT_REWIND_TITLE, TEXT_REWIND_EMPTY);
                return std::string(buf);
            }
            std::snprintf(buf, sizeof(buf), "%s | %ds", TEXT_REWIND_TITLE, getSelectionSecondsAgo());
            return std::string(buf);
        }

        int getSelectionSecondsAgo() const {
            if (selection < 0 || selection >= (int) entries.size()) {
                return 0;
            }
            return entries[selection].secondsAgo;
        }

    private:
        std::deque<RewindEntry> entries;
        int targetFps = 60;
        int sampleFrames = 120;
        int frameCounter = 0;
        int selection = -1;
        bool timelineVisible = false;
        bool comboLatch = false;
        bool selectionChanged = false;
        bool openStateValid = false;
        long long totalFramesCaptured = 0;
        std::vector<uint8_t> openStateData;

        uint32_t lastButtons = 0;
        std::chrono::steady_clock::time_point lastInputTime;
        int inputRepeatCount = 0;

        size_t maxMemoryBytes = 128ULL * 1024 * 1024;
        size_t currentMemoryBytes = 0;
        std::vector<double> timingsHistory;

        void captureSnapshot(
                const std::function<bool(std::vector<uint8_t> &)> &serializeFunc) {
            auto start = std::chrono::high_resolution_clock::now();

            RewindEntry entry;
            if (!serializeFunc(entry.stateData)) {
                return;
            }
            entry.frameOffset = totalFramesCaptured;

            // Memory management: evict oldest when over limit (O(1) with deque)
            size_t entrySize = entry.stateData.size();
            while (!entries.empty() && currentMemoryBytes + entrySize > maxMemoryBytes) {
                currentMemoryBytes -= entries.front().stateData.size();
                entries.pop_front();
            }
            entries.push_back(std::move(entry));
            currentMemoryBytes += entrySize;

            recalcSecondsAgo();

            if (timelineVisible) {
                selection = (int) entries.size() - 1;
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - start);
            recordTiming(elapsed.count() / 1000.0);
        }

        void recalcSecondsAgo() {
            if (entries.empty()) return;
            long long lastFrameOffset = entries.back().frameOffset;
            for (auto &entry : entries) {
                entry.secondsAgo = (int) ((lastFrameOffset - entry.frameOffset) / targetFps);
            }
        }

        bool openRewindTimeline(
                const std::function<bool(std::vector<uint8_t> &)> &serializeFunc,
                const std::function<bool(const std::vector<uint8_t> &)> &deserializeFunc,
                const std::function<void()> &clearInputFunc,
                const std::function<void()> &pauseFunc,
                const std::function<void()> &resumeFunc,
                const std::function<void()> &previewFunc,
                const std::function<void()> &clearAudioFunc,
                bool (*updateHintText)(void *userData, const char *text),
                void *userData) {
            timelineVisible = true;
            selection = std::max(0, (int) entries.size() - 1);
            selectionChanged = false;
            openStateData.clear();
            openStateValid = serializeFunc(openStateData);
            pauseFunc();
            clearInputFunc();
            if (updateHintText) {
                updateHintText(userData, getHintText().c_str());
            }
            return true;
        }

        void previewSelectedEntry(
                const std::function<bool(const std::vector<uint8_t> &)> &deserializeFunc) {
            if (selection >= 0 && selection < (int) entries.size()) {
                deserializeFunc(entries[selection].stateData);
            }
        }

        void previewSelectedEntryTransactional(
                const std::function<bool(const std::vector<uint8_t> &)> &deserializeFunc,
                const std::function<void()> &clearInputFunc,
                const std::function<void()> &previewFunc,
                const std::function<void()> &clearAudioFunc) {
            if (selection < 0 || selection >= (int) entries.size()) {
                return;
            }

            const auto &state = entries[selection].stateData;
            clearInputFunc();
            if (!deserializeFunc(state)) {
                return;
            }

            if (previewFunc) {
                previewFunc();
            }

            if (clearAudioFunc) {
                clearAudioFunc();
            }

            // Roll back to the selected savestate to avoid timeline drift.
            deserializeFunc(state);
            if (clearAudioFunc) {
                clearAudioFunc();
            }
        }
    };

}

#endif //C2DUI_REWIND_MANAGER_H
