const schedule = require('node-schedule');
const voting = require('./voting');

//定时配置 - pure 8 seconds
var singleJobSchedule = "*/8 * * * * *";

/**
 * @returns {Promise<void>}
 */
async function startJob() {

  schedule.scheduleJob(singleJobSchedule, function() {
    try {
      voting.votingRandomNum();
    } catch (exception) {
      console.error(`Execute 'voting.votingRandomNum' failed. Exception details: ${exception}`);
    }
  });
}

startJob();