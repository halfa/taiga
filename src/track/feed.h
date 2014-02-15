/*
** Taiga
** Copyright (C) 2010-2014, Eren Okka
** 
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TAIGA_TRACK_FEED_H
#define TAIGA_TRACK_FEED_H

#include <string>
#include <vector>

#include "library/anime_episode.h"
#include "taiga/http.h"

enum FeedItemState {
  FEEDITEM_BLANK,
  FEEDITEM_DISCARDED_NORMAL,
  FEEDITEM_DISCARDED_INACTIVE,
  FEEDITEM_DISCARDED_HIDDEN,
  FEEDITEM_SELECTED
};

class GenericFeedItem {
public:
  GenericFeedItem() : is_permalink(true) {}
  virtual ~GenericFeedItem() {}

  std::wstring title, link, description,
    author, category, comments, enclosure, guid, pub_date, source;
  bool is_permalink;
};

class FeedItem : public GenericFeedItem {
public:
  FeedItem() : index(-1), state(FEEDITEM_BLANK) {}
  virtual ~FeedItem() {};

  void Discard(int option);
  bool IsDiscarded() const;

  bool operator<(const FeedItem& item) const;

  int index;
  std::wstring magnet_link;
  FeedItemState state;

  class EpisodeData : public anime::Episode {
  public:
    EpisodeData() : new_episode(false) {}
    std::wstring file_size;
    bool new_episode;
  } episode_data;
};

////////////////////////////////////////////////////////////////////////////////

enum FeedCategory {
  // Broadcatching for torrent files and DDL
  FEED_CATEGORY_LINK,
  // News around the web
  FEED_CATEGORY_TEXT,
  // Airing times for anime titles
  FEED_CATEGORY_TIME
};

enum TorrentCategory {
  TORRENT_ANIME,
  TORRENT_BATCH,
  TORRENT_OTHER
};

class GenericFeed {
public:
  // Required channel elements
  std::wstring title, link, description;

  // Optional channel elements
  // (see www.rssboard.org/rss-specification for more information)
  /*
  std::wstring language, copyright, managingEditor, webMaster, pubDate, 
    lastBuildDate, category, generator, docs, cloud, ttl, image, 
    rating, textInput, skipHours, skipDays;
  */
  
  // Feed items
  std::vector<FeedItem> items;
};

class Feed : public GenericFeed {
public:
  Feed();
  virtual ~Feed() {}

  bool Check(const std::wstring& source, bool automatic = false);
  bool Download(int index);
  bool ExamineData();
  std::wstring GetDataPath();
  bool Load();

public:
  int category, download_index;
};

////////////////////////////////////////////////////////////////////////////////

enum FeedFilterElement {
  FEED_FILTER_ELEMENT_META_ID,
  FEED_FILTER_ELEMENT_META_STATUS,
  FEED_FILTER_ELEMENT_META_TYPE,
  FEED_FILTER_ELEMENT_META_EPISODES,
  FEED_FILTER_ELEMENT_META_DATE_START,
  FEED_FILTER_ELEMENT_META_DATE_END,
  FEED_FILTER_ELEMENT_USER_STATUS,
  FEED_FILTER_ELEMENT_LOCAL_EPISODE_AVAILABLE,
  FEED_FILTER_ELEMENT_EPISODE_TITLE,
  FEED_FILTER_ELEMENT_EPISODE_NUMBER,
  FEED_FILTER_ELEMENT_EPISODE_VERSION,
  FEED_FILTER_ELEMENT_EPISODE_GROUP,
  FEED_FILTER_ELEMENT_EPISODE_VIDEO_RESOLUTION,
  FEED_FILTER_ELEMENT_EPISODE_VIDEO_TYPE,
  FEED_FILTER_ELEMENT_FILE_TITLE,
  FEED_FILTER_ELEMENT_FILE_CATEGORY,
  FEED_FILTER_ELEMENT_FILE_DESCRIPTION,
  FEED_FILTER_ELEMENT_FILE_LINK,
  FEED_FILTER_ELEMENT_COUNT
};

enum FeedFilterOperator {
  FEED_FILTER_OPERATOR_EQUALS,
  FEED_FILTER_OPERATOR_NOTEQUALS,
  FEED_FILTER_OPERATOR_ISGREATERTHAN,
  FEED_FILTER_OPERATOR_ISGREATERTHANOREQUALTO,
  FEED_FILTER_OPERATOR_ISLESSTHAN,
  FEED_FILTER_OPERATOR_ISLESSTHANOREQUALTO,
  FEED_FILTER_OPERATOR_BEGINSWITH,
  FEED_FILTER_OPERATOR_ENDSWITH,
  FEED_FILTER_OPERATOR_CONTAINS,
  FEED_FILTER_OPERATOR_NOTCONTAINS,
  FEED_FILTER_OPERATOR_COUNT
};

enum FeedFilterMatch {
  FEED_FILTER_MATCH_ALL,
  FEED_FILTER_MATCH_ANY
};

class FeedFilterCondition {
public:
  FeedFilterCondition() : element(0), op(0) {}
  virtual ~FeedFilterCondition() {}
  FeedFilterCondition& operator=(const FeedFilterCondition& condition);

  void Reset();

public:
  int element;
  int op;
  std::wstring value;
};

enum FeedFilterAction {
  FEED_FILTER_ACTION_DISCARD,
  FEED_FILTER_ACTION_SELECT,
  FEED_FILTER_ACTION_PREFER
};

enum FeedFilterOption {
  FEED_FILTER_OPTION_DEFAULT,
  FEED_FILTER_OPTION_DEACTIVATE,
  FEED_FILTER_OPTION_HIDE
};

class FeedFilter {
public:
  FeedFilter() : action(0), enabled(true), match(FEED_FILTER_MATCH_ALL), option(FEED_FILTER_OPTION_DEFAULT) {}
  virtual ~FeedFilter() {}
  FeedFilter& operator=(const FeedFilter& filter);

  void AddCondition(int element, int op, const std::wstring& value);
  void Filter(Feed& feed, FeedItem& item, bool recursive);
  void Reset();

public:
  std::wstring name;
  bool enabled;
  int action, match, option;
  std::vector<int> anime_ids;
  std::vector<FeedFilterCondition> conditions;
};

class FeedFilterPreset {
public:
  FeedFilterPreset() : is_default(false) {}
  std::wstring description;
  FeedFilter filter;
  bool is_default;
};

enum FeedFilterShortcodeType {
  FEED_FILTER_SHORTCODE_ACTION,
  FEED_FILTER_SHORTCODE_ELEMENT,
  FEED_FILTER_SHORTCODE_MATCH,
  FEED_FILTER_SHORTCODE_OPERATOR,
  FEED_FILTER_SHORTCODE_OPTION
};

class FeedFilterManager {
public:
  FeedFilterManager();

  void InitializePresets();
  void InitializeShortcodes();

  void AddPresets();
  void AddFilter(int action, int match, int option, bool enabled, const std::wstring& name);
  void Cleanup();
  void Filter(Feed& feed, bool preferences);
  void FilterArchived(Feed& feed);
  bool IsItemDownloadAvailable(Feed& feed);
  void MarkNewEpisodes(Feed& feed);

  std::wstring CreateNameFromConditions(const FeedFilter& filter);
  std::wstring TranslateCondition(const FeedFilterCondition& condition);
  std::wstring TranslateConditions(const FeedFilter& filter, size_t index);
  std::wstring TranslateElement(int element);
  std::wstring TranslateOperator(int op);
  std::wstring TranslateValue(const FeedFilterCondition& condition);
  std::wstring TranslateMatching(int match);
  std::wstring TranslateAction(int action);
  std::wstring TranslateOption(int option);

  std::wstring GetShortcodeFromIndex(FeedFilterShortcodeType type, int index);
  int GetIndexFromShortcode(FeedFilterShortcodeType type, const std::wstring& shortcode);

public:
  std::vector<FeedFilter> filters;
  std::vector<FeedFilterPreset> presets;

private:
  std::map<int, std::wstring> action_shortcodes_;
  std::map<int, std::wstring> element_shortcodes_;
  std::map<int, std::wstring> match_shortcodes_;
  std::map<int, std::wstring> operator_shortcodes_;
  std::map<int, std::wstring> option_shortcodes_;
};

////////////////////////////////////////////////////////////////////////////////

class Aggregator {
public:
  Aggregator();
  virtual ~Aggregator() {}

  Feed* Get(int category);

  bool Notify(const Feed& feed);
  void ParseDescription(FeedItem& feed_item, const std::wstring& source);

  bool LoadArchive();
  bool SaveArchive();
  bool SearchArchive(const std::wstring& file);

private:
  bool CompareFeedItems(const GenericFeedItem& item1, const GenericFeedItem& item2);

public:
  std::vector<Feed> feeds;
  std::vector<std::wstring> file_archive;
  FeedFilterManager filter_manager;
};

extern Aggregator Aggregator;

#endif  // TAIGA_TRACK_FEED_H