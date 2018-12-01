#include <chrono>
#include <iostream>
#include <random>

#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <rpos/Proof.h>
#include <rpos/VoterSystem.h>
#include <rpos/Vrf.h>
#include <rpos/Seed.h>

using namespace ultrainio;

#define NODE_NUMBER 5000 // node number
#define M 60000 // min stake number for echo node
#define VOTER_TOTAL_COUNT 1000.0
#define PROPOSER_TOTAL_COUNT 20.0

struct NodeInfo {
    int stakes;
    int phase0ProposerVote;
    int phase0VoterVote;
    int phase1VoterVote;
    PrivateKey privateKey;
    PublicKey publicKey;
    Proof proof;
};

struct ExperimentStatistics {
    int phase0TotalProposerVoter;
    int phase0TotalVoterVote;
    int phase1TotalVoterVote;
    int phase0ProposerNum;
    int phase0VoterNum;
    int phase1VoterNum;
    int consumeMs;
};

static double calcVariance(ExperimentStatistics *statistics, size_t size, double mean, int which);

int main(int argc, char **argv) {
    VoterSystem voter;
    NodeInfo *nodeArray = new NodeInfo[NODE_NUMBER];
    for (int i = 0; i < NODE_NUMBER; i++) {
        nodeArray[i].stakes = M;
        nodeArray[i].phase0ProposerVote = 0;
        nodeArray[i].phase0VoterVote = 0;
        nodeArray[i].phase1VoterVote = 0;
        nodeArray[i].privateKey = PrivateKey::generate();
        nodeArray[i].publicKey = nodeArray[i].privateKey.getPublicKey();
        nodeArray[i].stakes = M;
    }
    double VOTER_RATIO = (VOTER_TOTAL_COUNT / NODE_NUMBER) / M;
    double PROPOSER_RATIO = (PROPOSER_TOTAL_COUNT / NODE_NUMBER) / M;

    int testCount = 100;
    ExperimentStatistics *statistics = new ExperimentStatistics[testCount];
    for (int i = 0; i < testCount; i++) {
        std::chrono::steady_clock::time_point pointStart = std::chrono::steady_clock::now();
        for (int j = 0; j < NODE_NUMBER; j++) {
            Seed seed(std::string("preHash"), 1545 + i, kPhaseBA0, 0);
            Proof proof = Vrf::vrf(nodeArray[j].privateKey, seed, Vrf::kProposer);
            nodeArray[j].phase0ProposerVote = voter.count(proof, nodeArray[j].stakes, PROPOSER_RATIO);

            Proof proofV = Vrf::vrf(nodeArray[j].privateKey, seed, Vrf::kVoter);
            nodeArray[j].phase0VoterVote = voter.count(proofV, nodeArray[j].stakes, VOTER_RATIO);

            Seed seedPhaseBA1(std::string("preHash"), 1545 + i, kPhaseBA1, 0);
            Proof proofV2 = Vrf::vrf(nodeArray[j].privateKey, seedPhaseBA1, Vrf::kVoter);
            nodeArray[j].phase1VoterVote = voter.count(proofV2, nodeArray[j].stakes, VOTER_RATIO);

            statistics[i].phase0TotalProposerVoter += nodeArray[j].phase0ProposerVote;
            statistics[i].phase0TotalVoterVote += nodeArray[j].phase0VoterVote;
            statistics[i].phase1TotalVoterVote += nodeArray[j].phase1VoterVote;
            if (nodeArray[j].phase0ProposerVote > 0) {
                statistics[i].phase0ProposerNum++;
            }
            if (nodeArray[j].phase0VoterVote > 0) {
                statistics[i].phase0VoterNum++;
            }
            if (nodeArray[j].phase1VoterVote > 0) {
                statistics[i].phase1VoterNum++;
            }
        }
        std::chrono::microseconds d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
        statistics[i].consumeMs = d.count();
    }
    int totalPhase0ProposerVote = 0;
    int totalPhase0VoterVote = 0;
    int totalPhase1VoterVote = 0;
    int totalPhase0ProposerNum = 0;
    int totalPhase0VoterNum = 0;
    int totalPhase1VoterNum = 0;
    double totalConsumeMs = 0.0;
    for (int i = 0; i < testCount; i++) {
        totalPhase0ProposerVote += statistics[i].phase0TotalProposerVoter;
        totalPhase0VoterVote += statistics[i].phase0TotalVoterVote;
        totalPhase1VoterVote += statistics[i].phase1TotalVoterVote;
        totalPhase0ProposerNum += statistics[i].phase0ProposerNum;
        totalPhase0VoterNum += statistics[i].phase0VoterNum;
        totalPhase1VoterNum += statistics[i].phase1VoterNum;
        totalConsumeMs += statistics[i].consumeMs;
    }
    double phase0ProposerMean = 0.0;
    double phase0ProposerVariance = 0.0;
    double phase0VoterMean = 0.0;
    double phase0VoterVariance = 0.0;
    double phase1VoterMean = 0.0;
    double phase1VoterVariance = 0.0;
    phase0ProposerMean = static_cast<double>(totalPhase0ProposerVote) / testCount;
    phase0VoterMean = static_cast<double>(totalPhase0VoterVote) / testCount;
    phase1VoterMean = static_cast<double>(totalPhase1VoterVote) / testCount;
    phase0ProposerVariance = calcVariance(statistics, testCount, phase0ProposerMean, 0);
    phase0VoterVariance = calcVariance(statistics, testCount, phase0VoterMean, 1);
    phase1VoterVariance = calcVariance(statistics, testCount, phase1VoterMean, 2);
    std::cout << "average phase0 proposer voter : " << phase0ProposerMean << " average num : " << totalPhase0ProposerNum * 1.0 / testCount << " variance : " << phase0ProposerVariance << std::endl
              << "average phase0 average voter : " << phase0VoterMean << " average num : " << totalPhase0VoterNum * 1.0 / testCount << " variance : " << phase0VoterVariance << std::endl
              << "average phase1 average voter : " << phase1VoterMean << " average num : " << totalPhase1VoterNum * 1.0 / testCount << " variance : " << phase1VoterVariance << std::endl;
    std::cout << "vote consume time : " << totalConsumeMs / (NODE_NUMBER * testCount * 3) << "ms" << std::endl;
    return 0;
}

static double calcVariance(ExperimentStatistics* statistics, size_t size, double mean, int which) {
    double total = 0.0;
    for (int i = 0; i < size; i++) {
        double value = 0.0;
        if (which == 0) {
            value = statistics[i].phase0TotalProposerVoter;
        } else if (which == 1) {
            value = statistics[i].phase0TotalVoterVote;
        } else if (which == 2) {
            value = statistics[i].phase1TotalVoterVote;
        }
        int diff = std::abs(value - (int)mean);
        total += std::pow(diff, 2);
    }
    return sqrt(total / size);
}
