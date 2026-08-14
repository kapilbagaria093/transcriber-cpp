#include <iostream>
#include <bits/stdc++.h>
#include <string>
#include <vector>

#include "whisper.h"
#include "common-whisper.h"

// tagging
#include <cstdio>
#include <fstream>
#include <sstream>

extern "C" {
    #include "wn.h"
}

// for setting env variables related to wordnet
#include <cstdlib>

// globals
std::vector<std::string> normalisedPOSTags = {
    "DT",
    "PRP",
    "NN",
    "J",
    "VB",
    "VBP",
    "VBG",
    "VBN",
    "MD",
    "RB",
    "WRB",
    "WP",
    "CD",
    "CC",
    "IN",
    "OTHER"
};


// structs
struct Token {
    std::string word;
    std::string spacyTag;
    std::string lemma;
};

struct NormalisedToken {
    std::string word;
    std::string tag; // tag as per ISL
    std::string lemma;
};

// declarations
int sentencePreProcessing(const std::string& sentence);
std::vector<Token> posTag(const std::string& sentence);
std::vector<NormalisedToken> normaliseTokensVector(const std::vector<Token>& tokensVector);
std::vector<NormalisedToken> eliminateEliminators(const std::vector<NormalisedToken>& tokens);

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

    // testing sentence preprocessing
    std::string testSentence = "this is a sentence.";
    sentencePreProcessing(testSentence);


    return EXIT_SUCCESS;
}

//Glad to see things are going well and business is starting to pick up. Andrea told me about your outstanding numbers on Tuesday. Keep up the good work. Now to other business, I am going to suggest a payment schedule for the outstanding monies that is due. One, can you pay the balance of the license agreement as soon as possible? Two, I suggest we setup or you suggest, what you can pay on the back royalties, would you feel comfortable with paying every two weeks? Every month, I will like to catch up and maintain current royalties. So, if we can start the current royalties and maintain them every two weeks as all stores are required to do, I would appreciate it. Let me know if this works for you.

/*
    NOW I HAVE THE ABILITY TO GENERATE WORDS FROM A SPEECH/AUDIOFILE AND I ALSO HAVE THE ABILITY TO RECOGNIZE WORDS AND THEIR TYPES AND STUFF

    NEXT STEPS: 
        1. PRE-PROCESSING OF SENTENCES (CONVERT SENTENCES TO THEIR REQUIRED FORM/GRAMMATICAL STRUCTURE OF ASL)
        2. USE WORDNET TO IDENTIFY THE WORDS AND CONVERT THEM TO THEIR BASE FORMS/FORM OF WORD FOR WHICH ANIMATION EXISTS
        3. FIND AND IMPLEMENT A 3D ANIMATION LIBRARY WHICH HAS ALL THE WORDS (ALL THE WORDS OF THE DICTIONARY -- TF (FIND SOLUTION))
*/

// isEliminator
bool isEliminator(const std::string& word) {
    static const std::unordered_set<std::string> eliminators = {
        "a", "an", "the",
        "be", "are",
        "and", "but", "so", "or", "yet"
    };

    return eliminators.count(word) > 0;
};

std::vector<Token> posTag(const std::string& sentence)
{
    // Temporary file containing the sentence
    const std::string inputFile = "pos_input.txt";

    {
        std::ofstream file(inputFile);

        if (!file) {
            std::cerr << "Failed to create POS input file\n";
            return {};
        }

        file << sentence;
    }

    // Run Python + spaCy
    const std::string command =
        ".venv/bin/python3 pos_tagger.py " + inputFile;

    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe) {
        std::cerr << "Failed to start spaCy POS tagger\n";
        return {};
    }

    std::vector<Token> tokens;

    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), pipe))
    {
        std::stringstream ss(buffer);

        Token token;

        std::getline(ss, token.word, '\t');
        std::getline(ss, token.spacyTag, '\t');
        std::getline(ss, token.lemma, '\t');

        if (!token.word.empty()) {
            tokens.push_back(token);
        }
    }

    int status = pclose(pipe);

    if (status != 0) {
        std::cerr << "spaCy POS tagger failed\n";
        return {};
    }

    return tokens;
}

std::vector<NormalisedToken> normaliseTokensVector(const std::vector<Token>& tokensVector) 
{
    std::vector<NormalisedToken> normalisedTokens;

    for (const auto& token : tokensVector) {

        NormalisedToken normalisedToken;

        normalisedToken.word = token.word;
        normalisedToken.lemma = token.lemma;

        const auto& spacyTag = token.spacyTag;

        // Determiner
        if (spacyTag == "DT") {
            normalisedToken.tag = "DT";
        }

        // Pronouns
        else if (spacyTag == "PRP" || spacyTag == "PRP$") {
            normalisedToken.tag = "PRP";
        }

        // Nouns
        else if (spacyTag == "NN"  ||
                 spacyTag == "NNS" ||
                 spacyTag == "NNP" ||
                 spacyTag == "NNPS") {
            normalisedToken.tag = "NN";
        }

        // Adjectives
        else if (spacyTag == "JJ"  ||
                 spacyTag == "JJR" ||
                 spacyTag == "JJS") {
            normalisedToken.tag = "J";
        }

        // Verbs
        else if (spacyTag == "VB"  ||
                 spacyTag == "VBD" ||
                 spacyTag == "VBZ") {
            normalisedToken.tag = "VB";
        }

        else if (spacyTag == "VBP") {
            normalisedToken.tag = "VBP";
        }

        else if (spacyTag == "VBG") {
            normalisedToken.tag = "VBG";
        }

        else if (spacyTag == "VBN") {
            normalisedToken.tag = "VBN";
        }

        // Modals
        else if (spacyTag == "MD") {
            normalisedToken.tag = "MD";
        }

        // Adverbs
        else if (spacyTag == "RB"  ||
                 spacyTag == "RBR" ||
                 spacyTag == "RBS") {
            normalisedToken.tag = "RB";
        }

        // Wh-adverbs
        else if (spacyTag == "WRB") {
            normalisedToken.tag = "WRB";
        }

        // Wh-pronouns
        else if (spacyTag == "WP" ||
                 spacyTag == "WP$") {
            normalisedToken.tag = "WP";
        }

        // Numbers
        else if (spacyTag == "CD") {
            normalisedToken.tag = "CD";
        }

        // Conjunctions
        else if (spacyTag == "CC") {
            normalisedToken.tag = "CC";
        }

        // Prepositions / subordinating conjunctions
        else if (spacyTag == "IN") {
            normalisedToken.tag = "IN";
        }

        // Unknown / unsupported
        else {
            normalisedToken.tag = "OTHER";
        }

        normalisedTokens.push_back(normalisedToken);
    }

    return normalisedTokens;
}

std::vector<NormalisedToken> eliminateEliminators(const std::vector<NormalisedToken>& tokens) 
{
    std::vector<NormalisedToken> result;

    for (const auto& token : tokens) {
        if (isEliminator(token.word) ||
            isEliminator(token.lemma)) {
            continue;
        }

        result.push_back(token);
    }

    return result;
}

std::vector<NormalisedToken> structureISLGrammar(const std::vector<NormalisedToken>& normalisedTokens)
{

}

// SENTENCE-WISE PREPROCESSING 
int sentencePreProcessing(const std::string& sentence) {
    auto tokensVector = posTag(sentence);

    for (const auto& token : tokensVector)
    {
        std::cout
            << token.word << "\t"
            << token.spacyTag << "\t"
            << token.lemma << "\n";
    };

    auto normalisedTokens = normaliseTokensVector(tokensVector);

    // no elimination yet
    // auto eliminatedTokens = eliminateEliminators(normalisedTokens);
    


    return EXIT_SUCCESS;
};






