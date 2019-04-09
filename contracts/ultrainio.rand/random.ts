import { Contract } from "ultrain-ts-lib/src/contract";
import {
  // Voter,
  // Waiter,
  RandRecord,
  // VOTER_TABLE,
  // WAITER_TABLE,
  // RAND_TABLE,
  Random
} from "./lib/random.lib";


class RandApp extends Contract {

  random: Random;
  constructor(code: u64){
    super(code);
    this.random = new Random();
  }

  @action
  queryLatest(): RandRecord {
    return this.random.queryLatest();
  }

  @action
  query(bckNum: u64): RandRecord {
    return this.random.query(bckNum);
  }
}
