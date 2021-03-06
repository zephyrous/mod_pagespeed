/*
 * Copyright 2014 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Author: jmaessen@google.com (Jan-Willem Maessen)

#ifndef NET_INSTAWEB_REWRITER_PUBLIC_MOBILIZE_DECISION_TREES_H_
#define NET_INSTAWEB_REWRITER_PUBLIC_MOBILIZE_DECISION_TREES_H_

#include "net/instaweb/rewriter/public/decision_tree.h"

namespace net_instaweb {

namespace MobileRole {
  enum Level {
    // Tags which aren't explicitly tagged with a data-mobile-role attribute,
    // but we want to keep anyway, such as <style> or <script> tags in the body.
    kKeeper = 0,
    // The page header, such as <h1> or logos.
    kHeader,
    // Nav sections of the page. The HTML of nav blocks will be completely
    // rewritten to be mobile friendly.
    kNavigational,
    // Main content of the page.
    kContent,
    // Any block that isn't one of the above. Marginal content is put at the end
    // and otherwise remains pretty much untouched with respect to modifying
    // HTML or styling.
    kMarginal,
    // Elements below don't have a defined role (the order matters).
    // Elements that we've decided should not have a data-mobile-role attribute
    // will be kInvalid.
    kInvalid,
    // Elements whose data-mobile-role is still undecided will be kUnassigned.
    // This value must not exist after labeling.
    kUnassigned
  };
}  // namespace MobileRole

// The following three enums are used to name signals used by the decision trees
// and computed by the mobilize_label_filter.

// Tags that are considered relevant and are counted in a sample.  Some tags are
// role tags or otherwise considered div-like.  These tag names are used to
// index the RelevantTagCount and RelevantTagPercent features below.
// Note that it's possible to add new tags to this list.
enum MobileRelevantTag {
  kATag = 0,
  kArticleTag,
  kAsideTag,
  kButtonTag,
  kContentTag,
  kDatalistTag,  // Useless?
  kDivTag,
  kFieldsetTag,
  kFooterTag,
  kFormTag,
  kH1Tag,
  kH2Tag,
  kH3Tag,
  kH4Tag,
  kH5Tag,
  kH6Tag,
  kHeaderTag,
  kImgTag,
  kInputTag,
  kLegendTag,    // Useless?
  kLiTag,
  kMainTag,
  kMenuTag,
  kNavTag,
  kOptgroupTag,  // Useless?
  kOptionTag,
  kPTag,
  kSectionTag,
  kSelectTag,    // Useless?
  kSpanTag,
  kTextareaTag,
  kUlTag,
  kNumRelevantTags
};

// Attribute substrings that are considered interesting if they occur in the id,
// class, or role of a div-like tag.
enum MobileAttrSubstring {
  kArticleAttr = 0,  // Useless?
  kAsideAttr,        // Useless?
  kBannerAttr,
  kBarAttr,
  kBodyAttr,         // Useless?
  kBotAttr,
  kCenterAttr,       // Useless?
  kColAttr,
  kCommentAttr,
  kContentAttr,
  kFindAttr,         // Useless?
  kFootAttr,
  kHdrAttr,          // Useless?
  kHeadAttr,
  kLeftAttr,         // Useless?
  kLogoAttr,
  kMainAttr,         // Useless?
  kMarginAttr,       // Useless?
  kMenuAttr,
  kMidAttr,
  kNavAttr,
  kPostAttr,
  kRightAttr,        // Useless?
  kSearchAttr,
  kSecAttr,
  kTitleAttr,        // Useless?
  kTopAttr,
  kWrapAttr,
  kNumAttrStrings
};

// Every feature has a symbolic name given by Name or Name + Index.
// DEFINITIONS OF FEATURES:
// * "Previous" features do not include the tag being labeled.
// * "Contained" and "Relevant" features do include the tag being labeled.
// * "TagCount" features ignore clearly non-user-visible tags such as <script>,
//   <style>, and <link>, and include only tags inside <body>.
// * "TagDepth" features include only div-like tags such as <div>, <section>,
//   <header>, and <aside> (see kRoleTags and kDivLikeTags in
//   mobilize_label_filter.cc).  They are the nesting depth of the tag within
//   <body>.
// * ElementTagDepth is the depth of the tag being sampled itself.
// * ContainedTagDepth is the maximum depth of any div-like child of this tag.
// * ContainedTagRelativeDepth is the difference between these two depths.
// * ContentBytes Ignores tags and their attributes, and also ignores leading
//   and trailing whitespace between tags.  So "hi there" is 8 ContentBytes,
//   but "hi <i class='foo'>there</i>" is only 7 ContentBytes.
// * NonBlankBytes is like ContentBytes but ignores all whitespace.
// * HasAttrString is a family of 0/1 entries indicating whether the
//   corresponding string (see kRelevantAttrSubstrings in
//   mobilize_label_filter.cc) occurs in the class, id, or role attribute of the
//   sampled tag.
// * RelevantTagCount is a series of counters indicating the number of various
//   "interesting" HTML tags within the current tag.  This includes all div-like
//   tags along with tags such as <p>, <a>, <h1>, and <img> (see kRelevantTags
//   in mobilize_label_filter.cc).
// * ContainedA / ContainedNonA statistics count occurrences that are nested
//   inside <a> tags vs not nested inside <a> tags.  LocalPercent is the ratio
//   of ContainedA / (ContainedA+ContainedNonA).
// * ParentRoleIs tracks the role of the parent of an element.  Only
//   kNavigational, kContent, and kHeader are classified; kMarginal is only
//   assigned after the fact to nodes with no otherwise-classified children.  We
//   actually train the classifiers looking only at the signals *different* from
//   the signal being trained, since parent->child propagation will assign the
//   parent's role to us automatically if no other role is assigned.
enum FeatureName {
  kElementTagDepth = 0,
  kPreviousTagCount,
  kPreviousTagPercent,
  kPreviousContentBytes,
  kPreviousContentPercent,
  kPreviousNonBlankBytes,
  kPreviousNonBlankPercent,
  kContainedTagDepth,
  kContainedTagRelativeDepth,
  kContainedTagCount,
  kContainedTagPercent,
  kContainedContentBytes,
  kContainedContentPercent,
  kContainedNonBlankBytes,
  kContainedNonBlankPercent,
  kContainedAContentBytes,
  kContainedNonAContentBytes,
  kContainedAContentLocalPercent,
  kContainedAImgTag,
  kContainedNonAImgTag,
  kContainedAImgLocalPercent,
  kHasAttrString,
  kRelevantTagCount = kHasAttrString + kNumAttrStrings,
  kRelevantTagPercent = kRelevantTagCount + kNumRelevantTags,
  kParentRoleIs = kRelevantTagPercent + kNumRelevantTags,
  kNumFeatures = kParentRoleIs + MobileRole::kMarginal
};

// Forward declarations for the machine-generated decision trees used by
// mobilize_label_filter to classify DOM elements.

extern const net_instaweb::DecisionTree::Node kNavigationalTree[];
extern const double kNavigationalTreeThreshold;
extern const int kNavigationalTreeSize;

extern const net_instaweb::DecisionTree::Node kHeaderTree[];
extern const double kHeaderTreeThreshold;
extern const int kHeaderTreeSize;

extern const net_instaweb::DecisionTree::Node kContentTree[];
extern const double kContentTreeThreshold;
extern const int kContentTreeSize;

}  // namespace net_instaweb

#endif  // NET_INSTAWEB_REWRITER_PUBLIC_MOBILIZE_DECISION_TREES_H_
