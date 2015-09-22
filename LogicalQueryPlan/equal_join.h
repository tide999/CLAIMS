/*
 * Copyright [2012-2015] DaSE@ECNU
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * /CLAIMS/LogicalQueryPlan/equal_join.h
 *
 *  Created on: Nov 21, 2013
 *      Author: wangli
 *       Email:
 *
 * Description:
 *   This file mainly describe the EqualJoin Operater.
 *
 */
#ifndef LOGICALQUERYPLAN_EQUAL_JOIN_H_
#define LOGICALQUERYPLAN_EQUAL_JOIN_H_
#include <vector>
#include "LogicalOperator.h"
#include "../Catalog/Attribute.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif
#include "../BlockStreamIterator/ParallelBlockStreamIterator/BlockStreamSortIterator.h"
/***
 * @brief The LogicalSort contains the information of attributes to be
 * joined.And it describe how to generate 'Join Operator'.
 * @details EqualJoin operator corresponding to the join statement to join two
 *tables.Require equivalent conditions.As for implementation, one of it is to
 *send all data to a same machine.But the table generated will be too big to
 *store and it is inefficient.So we partition the data to several machines.Each
 *of them do some join and return.
 *
 */
class EqualJoin : public LogicalOperator {
 public:
  struct JoinPair {
    JoinPair(const Attribute& a, const Attribute& b) : first(a), second(b) {}
    Attribute first;
    Attribute second;
  };
  enum JoinPolice {
    na,
    no_repartition,
    left_repartition,
    right_repartition,
    complete_repartition
  };

 public:
  EqualJoin(std::vector<JoinPair>, LogicalOperator* left_input,
            LogicalOperator* right_input);
  virtual ~EqualJoin();
  Dataflow getDataflow();
  BlockStreamIteratorBase* getIteratorTree(const unsigned& blocksize);
  bool GetOptimalPhysicalPlan(Requirement requirement,
                              PhysicalPlanDescriptor& physical_plan_descriptor,
                              const unsigned& block_size = 4096 * 1024);

 private:
  std::vector<unsigned> getLeftJoinKeyIndexList() const;
  std::vector<unsigned> getRightJoinKeyIndexList() const;
  std::vector<unsigned> getLeftPayloadIndexList() const;
  std::vector<unsigned> getRightPayloadIndexList() const;
  int getIndexInLeftJoinKeyList(const Attribute&) const;
  int getIndexInLeftJoinKeyList(
      const Attribute&,
      const std::vector<Attribute> shadow_attribute_list) const;
  int getIndexInRightJoinKeyList(const Attribute&) const;
  int getIndexInRightJoinKeyList(
      const Attribute&,
      const std::vector<Attribute> shadow_attribute_list) const;
  int getIndexInAttributeList(const std::vector<Attribute>& attributes,
                              const Attribute&) const;
  bool isHashOnLeftKey(const Partitioner& part, const Attribute& key) const;

  /*check whether the partitioning is based on hash and the hash key is a subset
   * of the join keys such that hash join
   * is enabled.
   */
  bool canLeverageHashPartition(
      const std::vector<Attribute>& partition_key_list,
      const DataflowPartitioningDescriptor& partitoiner) const;

  bool isEqualCondition(const Attribute& a1, const Attribute& a2) const;

  /** current version only consider the data size for simplicity.
   * TODO: consider not only data size but also other factors, such as
   * parallelism, resource, etc.**/
  JoinPolice decideLeftOrRightRepartition(const Dataflow& left_dataflow,
                                          const Dataflow& right_dataflow) const;

  DataflowPartitioningDescriptor decideOutputDataflowProperty(
      const Dataflow& left_dataflow, const Dataflow& right_dataflow) const;
  void print(int level = 0) const;

  /**
   * assuming that R and S are the two join table, the selectivity is the number
   * of tuples generated by the join operator to the number of |R|*|S|.
   */
  double predictEqualJoinSelectivity(const Dataflow& left_dataflow,
                                     const Dataflow& right_dataflow) const;

  /**
   * assuming that R ane S are the two join table, and the join condition is
   * R.x=S.x.
   * return |O|, where |O|=|R.x=x1|*|S.x=x1|+|R.x=x2|*|S.x=x2|+......
   */
  double predictEqualJoinSelectivityOnSingleJoinAttributePair(
      const Attribute& a_l, const Attribute& a_r) const;

 private:
  std::vector<JoinPair> joinkey_pair_list_;
  std::vector<Attribute> left_join_key_list_;
  std::vector<Attribute> right_join_key_list_;
  LogicalOperator* left_child_;
  LogicalOperator* right_child_;
  JoinPolice join_police_;
  Dataflow* dataflow_;
};

#endif  // LOGICALQUERYPLAN_EQUAL_JOIN_H_
