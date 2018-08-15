#include <chrono>
#include <iostream>
#include <random>

#include "uranus/Voter.h"
#include "crypto/Security.h"

using namespace ultrainio;

struct NodeInfo {
    int stakes;
    int phase0ProposerVote;
    int phase0VoterVote;
    int phase1VoterVote;
    uint8_t sk[VRF_PRIVATE_KEY_LEN];
    uint8_t pk[VRF_PUBLIC_KEY_LEN];
    uint8_t proof[VRF_PROOF_LEN];
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
    Voter voter;
    int nodeNum = 1000;
    NodeInfo *nodeArray = new NodeInfo[nodeNum];
    std::default_random_engine generator;
    std::gamma_distribution<double> distribution(1.8, 0.5);
    int totalGammaStakes = 0;
    for (int i = 0; i < nodeNum; i++) {
        nodeArray[i].stakes = distribution(generator) * 100;
        nodeArray[i].phase0ProposerVote = 0;
        nodeArray[i].phase0VoterVote = 0;
        nodeArray[i].phase1VoterVote = 0;
        security::vrf_keypair(nodeArray[i].pk, nodeArray[i].sk);
        totalGammaStakes += nodeArray[i].stakes;
    }
    for (int i = 0; i < nodeNum; i++) {
        nodeArray[i].stakes = nodeArray[i].stakes * Voter::TOTAL_STAKES / totalGammaStakes;
        //std::cout << " node [" << i << "] stakes = " << nodeArray[i].stakes << std::endl;
    }

    int testCount = 100;
    ExperimentStatistics *statistics = new ExperimentStatistics[testCount];
    for (int i = 0; i < testCount; i++) {
        std::chrono::steady_clock::time_point pointStart = std::chrono::steady_clock::now();
        for (int j = 0; j < nodeNum; j++) {
            std::string round = std::to_string(1545 + i);
            std::string phase0("00");
            uint8_t proof[VRF_PROOF_LEN];
            nodeArray[j].phase0ProposerVote = voter.vote(round + phase0 + "01", nodeArray[j].sk,
                                                         nodeArray[j].stakes, Voter::PROPOSER_RATIO, proof);
            nodeArray[j].phase0VoterVote = voter.vote(round + phase0 + "02", nodeArray[j].sk,
                                                      nodeArray[j].stakes, Voter::VOTER_RATIO, proof);
            std::string phase1("01");
            nodeArray[j].phase1VoterVote = voter.vote(round + phase1 + "01", nodeArray[j].sk,
                                                      nodeArray[j].stakes, Voter::VOTER_RATIO, proof);
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
    std::cout << "vote consume time : " << totalConsumeMs / (nodeNum * testCount * 3) << "ms" << std::endl;
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
