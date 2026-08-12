#include <iostream>
#include <string>
#include <vector>

#include "whisper.h"
#include "common-whisper.h"

extern "C" {
    #include "wn.h"
}

// for setting env variables related to wordnet
#include <cstdlib>

int main()
{
    // wordnet env variable
    setenv(
        "WNSEARCHDIR",
        "third_party/wordnet/dict",
        1
    );

    const std::string audioPath =
        "audio/test.wav";

    const std::string modelPath =
        "models/ggml-base.bin";

    // 1. Load audio
    std::vector<float> audio;
    std::vector<std::vector<float>> audioStereo;

    if (!read_audio_data(
            audioPath,
            audio,
            audioStereo,
            false))
    {
        std::cerr
            << "Failed to load audio: "
            << audioPath
            << '\n';

        return 1;
    }

    std::cout
        << "Loaded audio: "
        << audio.size()
        << " samples\n";


    // 2. Load Whisper model
    whisper_context_params contextParams =
        whisper_context_default_params();

    whisper_context* ctx =
        whisper_init_from_file_with_params(
            modelPath.c_str(),
            contextParams
        );

    if (!ctx)
    {
        std::cerr
            << "Failed to load Whisper model\n";

        return 1;
    }


    // 3. Configure Whisper
    whisper_full_params params =
        whisper_full_default_params(
            WHISPER_SAMPLING_GREEDY
        );

    params.print_progress = true;
    params.print_realtime = false;
    params.print_timestamps = true;

    params.language = "auto";


    // 4. Run transcription
    std::cout << "Transcribing...\n";

    int result = whisper_full(
        ctx,
        params,
        audio.data(),
        audio.size()
    );

    if (result != 0)
    {
        std::cerr
            << "Whisper transcription failed\n";

        whisper_free(ctx);

        return 1;
    }


    // 5. Get transcription
    std::vector<std::string> transcriptionStrings;
    std::string transcription;

    const int segmentCount =
        whisper_full_n_segments(ctx);

    for (int i = 0; i < segmentCount; ++i)
    {
        const char* text =
            whisper_full_get_segment_text(
                ctx,
                i
            );

        if (text) {
            transcription += text;
        }
    };

    // Split into sentences
    std::string currentSentence;
    std::vector<std::string> transcriptions;
    for (char c : transcription) {
        currentSentence += c;

        if (c == '?' || c == '!' || c == '.' || c == ':') {
            std::cout << "------------------------------------\n";
            std::cout << currentSentence << "\n";
            std::cout << "------------------------------------\n";
            transcriptions.push_back(currentSentence);
            currentSentence.clear();
        }
    }

    // 7. Cleanup
    whisper_free(ctx);

    return EXIT_SUCCESS;
}

//Glad to see things are going well and business is starting to pick up. Andrea told me about your outstanding numbers on Tuesday. Keep up the good work. Now to other business, I am going to suggest a payment schedule for the outstanding monies that is due. One, can you pay the balance of the license agreement as soon as possible? Two, I suggest we setup or you suggest, what you can pay on the back royalties, would you feel comfortable with paying every two weeks? Every month, I will like to catch up and maintain current royalties. So, if we can start the current royalties and maintain them every two weeks as all stores are required to do, I would appreciate it. Let me know if this works for you.