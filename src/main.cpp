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

extern "C"
{
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
    "OTHER"};

// structs
struct Token
{
    // Original lexical information
    std::string word;
    std::string lemma;

    // Part of speech
    std::string pos;
    std::string tag;

    // Morphological information
    std::string morphology;

    // Dependency information
    std::string dependency;
    int headIndex;

    // Named entity information
    std::string entityType;
    std::string entityIOB;

    // Position in original sentence
    int index;
    int characterOffset;

    // Useful lexical properties
    bool isNumber;
    bool isPunctuation;
};

enum class PhraseRole
{
    SUBJECT,
    OBJECT,
    INDIRECT_OBJECT,
    COMPLEMENT,
    OTHER
};

struct Phrase
{
    PhraseRole role;

    int headIndex;

    std::vector<int> tokenIndices;
};

// declarations
int sentencePreProcessing(const std::string &sentence);
std::vector<Token> posTag(const std::string &sentence);

int main()
{
    // wordnet env variable
    setenv(
        "WNSEARCHDIR",
        "third_party/wordnet/dict",
        1);

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

    whisper_context *ctx =
        whisper_init_from_file_with_params(
            modelPath.c_str(),
            contextParams);

    if (!ctx)
    {
        std::cerr
            << "Failed to load Whisper model\n";

        return 1;
    }

    // 3. Configure Whisper
    whisper_full_params params =
        whisper_full_default_params(
            WHISPER_SAMPLING_GREEDY);

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
        audio.size());

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
        const char *text =
            whisper_full_get_segment_text(
                ctx,
                i);

        if (text)
        {
            transcription += text;
        }
    };

    // Split into sentences
    std::string currentSentence;
    std::vector<std::string> transcriptions;
    for (char c : transcription)
    {
        currentSentence += c;

        if (c == '?' || c == '!' || c == '.' || c == ':')
        {
            // std::cout << "------------------------------------\n";
            // std::cout << currentSentence << "\n";
            // std::cout << "------------------------------------\n";
            transcriptions.push_back(currentSentence);
            currentSentence.clear();
        }
    }

    // 7. Cleanup
    whisper_free(ctx);

    // testing sentence preprocessing -- at this point, we have speech broken down into sentences
    std::string testSentence = "The young boy eats an apple.";
    sentencePreProcessing(testSentence);

    return EXIT_SUCCESS;
};

// Glad to see things are going well and business is starting to pick up. Andrea told me about your outstanding numbers on Tuesday. Keep up the good work. Now to other business, I am going to suggest a payment schedule for the outstanding monies that is due. One, can you pay the balance of the license agreement as soon as possible? Two, I suggest we setup or you suggest, what you can pay on the back royalties, would you feel comfortable with paying every two weeks? Every month, I will like to catch up and maintain current royalties. So, if we can start the current royalties and maintain them every two weeks as all stores are required to do, I would appreciate it. Let me know if this works for you.

/*
    NOW I HAVE THE ABILITY TO GENERATE WORDS FROM A SPEECH/AUDIOFILE AND I ALSO HAVE THE ABILITY TO RECOGNIZE WORDS AND THEIR TYPES AND STUFF

    NEXT STEPS:
        1. PRE-PROCESSING OF SENTENCES (CONVERT SENTENCES TO THEIR REQUIRED FORM/GRAMMATICAL STRUCTURE OF ASL)
        2. USE WORDNET TO IDENTIFY THE WORDS AND CONVERT THEM TO THEIR BASE FORMS/FORM OF WORD FOR WHICH ANIMATION EXISTS
        3. FIND AND IMPLEMENT A 3D ANIMATION LIBRARY WHICH HAS ALL THE WORDS (ALL THE WORDS OF THE DICTIONARY -- TF (FIND SOLUTION))
*/

std::vector<Token> posTag(const std::string &sentence)
{
    // Temporary file containing the sentence
    const std::string inputFile = "pos_input.txt";

    {
        std::ofstream file(inputFile);

        if (!file)
        {
            std::cerr << "Failed to create POS input file\n";
            return {};
        }

        file << sentence;
    }

    // Run Python + spaCy
    const std::string command =
        ".venv/bin/python3 pos_tagger.py " + inputFile;

    FILE *pipe = popen(command.c_str(), "r");

    if (!pipe)
    {
        std::cerr << "Failed to start spaCy POS tagger\n";
        return {};
    }

    std::vector<Token> tokens;

    char buffer[4096];

    while (fgets(buffer, sizeof(buffer), pipe))
    {
        std::stringstream ss(buffer);

        Token token;

        // Lexical information
        std::getline(ss, token.word, '\t');
        std::getline(ss, token.lemma, '\t');

        // POS information
        std::getline(ss, token.pos, '\t');
        std::getline(ss, token.tag, '\t');

        // Morphology
        std::getline(ss, token.morphology, '\t');

        // Dependency information
        std::getline(ss, token.dependency, '\t');

        std::string headIndex;
        std::getline(ss, headIndex, '\t');
        token.headIndex = std::stoi(headIndex);

        // Named entity information
        std::getline(ss, token.entityType, '\t');
        std::getline(ss, token.entityIOB, '\t');

        // Position information
        std::string index;
        std::getline(ss, index, '\t');
        token.index = std::stoi(index);

        std::string characterOffset;
        std::getline(ss, characterOffset, '\t');
        token.characterOffset = std::stoi(characterOffset);

        // Useful properties
        std::string isNumber;
        std::getline(ss, isNumber, '\t');
        token.isNumber = (isNumber == "1");

        std::string isPunctuation;
        std::getline(ss, isPunctuation, '\t');
        token.isPunctuation = (isPunctuation == "1");

        if (!token.word.empty())
        {
            tokens.push_back(token);
        }
    }

    int status = pclose(pipe);

    if (status != 0)
    {
        std::cerr << "spaCy POS tagger failed\n";
        return {};
    }

    return tokens;
}

// build dependency tree
int findRoot(const std::vector<Token> &tokens)
{
    for (const auto &token : tokens)
    {
        if (token.dependency == "ROOT")
            return token.index;
    }

    return -1;
};

// unordered map stores children of each index. (like a graph represented by list)
std::unordered_map<int, std::vector<int>>
buildDependencyTree(const std::vector<Token> &tokens)
{
    std::unordered_map<int, std::vector<int>> children;

    for (const auto &token : tokens)
    {
        // Root points to itself in spaCy
        if (token.index == token.headIndex)
            continue;

        children[token.headIndex].push_back(token.index);
    }

    return children;
};

void printDependencyTree(
    const std::vector<Token> &tokens,
    const std::unordered_map<int, std::vector<int>> &children,
    int tokenIndex,
    int depth = 0)
{
    const Token &token = tokens[tokenIndex];

    for (int i = 0; i < depth; ++i)
    {
        std::cout << "    ";
    }

    std::cout
        << token.word
        << " ["
        << token.dependency
        << "]"
        << "\n";

    auto it = children.find(tokenIndex);

    if (it == children.end())
        return;

    for (int childIndex : it->second)
    {
        printDependencyTree(
            tokens,
            children,
            childIndex,
            depth + 1);
    }
};

PhraseRole getPhraseRole(const Token& token)
{
    // SUBJECT
    if (token.dependency == "nsubj" ||
        token.dependency == "nsubjpass")
    {
        return PhraseRole::SUBJECT;
    }

    // OBJECT
    if (token.dependency == "obj" ||
        token.dependency == "dobj")
    {
        return PhraseRole::OBJECT;
    }

    // INDIRECT OBJECT
    if (token.dependency == "iobj")
    {
        return PhraseRole::INDIRECT_OBJECT;
    }

    // COMPLEMENT
    if (token.dependency == "attr" ||
        token.dependency == "acomp" ||
        token.dependency == "xcomp" ||
        token.dependency == "ccomp")
    {
        return PhraseRole::COMPLEMENT;
    }

    return PhraseRole::OTHER;
}

std::vector<int> getSubtree(
    const std::unordered_map<int, std::vector<int>> &children,
    int rootIndex)
{
    std::vector<int> result;

    result.push_back(rootIndex);

    auto it = children.find(rootIndex);

    if (it == children.end())
        return result;

    for (int childIndex : it->second)
    {
        std::vector<int> childSubtree =
            getSubtree(children, childIndex);

        result.insert(
            result.end(),
            childSubtree.begin(),
            childSubtree.end());
    }

    return result;
}

std::vector<Phrase> extractPhrases(
    const std::vector<Token> &tokens,
    const std::unordered_map<int, std::vector<int>> &children,
    int rootIndex)
{
    std::vector<Phrase> phrases;

    auto it = children.find(rootIndex);

    if (it == children.end())
        return phrases;

    for (int childIndex : it->second)
    {
        const Token &child = tokens[childIndex];

        PhraseRole role = getPhraseRole(child);

        if (role == PhraseRole::OTHER)
            continue;

        Phrase phrase;

        phrase.role = role;
        phrase.headIndex = childIndex;

        phrase.tokenIndices =
            getSubtree(children, childIndex);

        std::sort(
            phrase.tokenIndices.begin(),
            phrase.tokenIndices.end());

        phrases.push_back(phrase);
    }

    return phrases;
}

// SENTENCE-WISE PREPROCESSING
int sentencePreProcessing(const std::string &sentence)
{
    auto tokensVector = posTag(sentence);

    // for (const auto &token : tokensVector)
    // {
    //     std::cout
    //         << "----------------------------------------\n"
    //         << "Word:              " << token.word << "\n"
    //         << "Lemma:             " << token.lemma << "\n"
    //         << "POS:               " << token.pos << "\n"
    //         << "Tag:               " << token.tag << "\n"
    //         << "Morphology:        " << token.morphology << "\n"
    //         << "Dependency:        " << token.dependency << "\n"
    //         << "Head Index:        " << token.headIndex << "\n"
    //         << "Entity Type:       " << token.entityType << "\n"
    //         << "Entity IOB:        " << token.entityIOB << "\n"
    //         << "Token Index:       " << token.index << "\n"
    //         << "Character Offset:  " << token.characterOffset << "\n"
    //         << "Is Number:         " << std::boolalpha << token.isNumber << "\n"
    //         << "Is Punctuation:    " << std::boolalpha << token.isPunctuation << "\n";
    // }

    // std::cout << "----------------------------------------\n";

    int root = findRoot(tokensVector);

    if (root != -1)
    {
        auto children = buildDependencyTree(tokensVector);
        std::cout << "----------------------------------------\n";
        printDependencyTree(
            tokensVector,
            children,
            root);

        std::cout << "----------------------------------------\n";

        auto subtree = getSubtree(children, root);

        auto phrases = extractPhrases(tokensVector, children, root);

        // printing phrases
        for (const auto &phrase : phrases)
        {
            std::cout << "\nPhrase: ";

            switch (phrase.role)
            {
            case PhraseRole::SUBJECT:
                std::cout << "SUBJECT";
                break;

            case PhraseRole::OBJECT:
                std::cout << "OBJECT";
                break;

            case PhraseRole::INDIRECT_OBJECT:
                std::cout << "INDIRECT OBJECT";
                break;

            case PhraseRole::COMPLEMENT:
                std::cout << "COMPLEMENT";
                break;

            default:
                std::cout << "OTHER";
            }

            std::cout << "\nHead: "
                      << tokensVector[phrase.headIndex].word
                      << "\nWords: ";

            for (int index : phrase.tokenIndices)
            {
                std::cout << tokensVector[index].word << " ";
            }

            std::cout << "\n";
        }
    }

    // give me the subtree of tokooen which has index 1 (root word)
    // subtree shows

    return EXIT_SUCCESS;
};
