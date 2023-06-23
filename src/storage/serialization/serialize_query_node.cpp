// GENERATED BY scripts/generate_serialization.py

#include "duckdb/common/serializer/format_serializer.hpp"
#include "duckdb/common/serializer/format_deserializer.hpp"
#include "duckdb/parser/query_node/list.hpp"

namespace duckdb {

void QueryNode::FormatSerialize(FormatSerializer &serializer) const {
	serializer.WriteProperty("type", type);
	serializer.WriteProperty("modifiers", modifiers);
	serializer.WriteProperty("cte_map", cte_map);
}

unique_ptr<QueryNode> QueryNode::FormatDeserialize(FormatDeserializer &deserializer) {
	auto type = deserializer.ReadProperty<QueryNodeType>("type");
	auto modifiers = deserializer.ReadProperty<vector<unique_ptr<ResultModifier>>>("modifiers");
	auto cte_map = deserializer.ReadProperty<CommonTableExpressionMap>("cte_map");
	unique_ptr<QueryNode> result;
	switch (type) {
	case QueryNodeType::RECURSIVE_CTE_NODE:
		result = RecursiveCTENode::FormatDeserialize(deserializer);
		break;
	case QueryNodeType::SELECT_NODE:
		result = SelectNode::FormatDeserialize(deserializer);
		break;
	case QueryNodeType::SET_OPERATION_NODE:
		result = SetOperationNode::FormatDeserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of QueryNode!");
	}
	result->modifiers = std::move(modifiers);
	result->cte_map = std::move(cte_map);
	return result;
}

void SelectNode::FormatSerialize(FormatSerializer &serializer) const {
	QueryNode::FormatSerialize(serializer);
	serializer.WriteProperty("select_list", select_list);
	serializer.WriteOptionalProperty("from_table", from_table);
	serializer.WriteOptionalProperty("where_clause", where_clause);
	serializer.WriteProperty("group_expressions", groups.group_expressions);
	serializer.WriteProperty("grouping_sets", groups.grouping_sets);
	serializer.WriteProperty("aggregate_handling", aggregate_handling);
	serializer.WriteOptionalProperty("having", having);
	serializer.WriteOptionalProperty("sample", sample);
	serializer.WriteOptionalProperty("qualify", qualify);
}

unique_ptr<QueryNode> SelectNode::FormatDeserialize(FormatDeserializer &deserializer) {
	auto result = make_uniq<SelectNode>();
	deserializer.ReadProperty("select_list", result->select_list);
	deserializer.ReadOptionalProperty("from_table", result->from_table);
	deserializer.ReadOptionalProperty("where_clause", result->where_clause);
	deserializer.ReadProperty("group_expressions", result->groups.group_expressions);
	deserializer.ReadProperty("grouping_sets", result->groups.grouping_sets);
	deserializer.ReadProperty("aggregate_handling", result->aggregate_handling);
	deserializer.ReadOptionalProperty("having", result->having);
	deserializer.ReadOptionalProperty("sample", result->sample);
	deserializer.ReadOptionalProperty("qualify", result->qualify);
	return std::move(result);
}

void SetOperationNode::FormatSerialize(FormatSerializer &serializer) const {
	QueryNode::FormatSerialize(serializer);
	serializer.WriteProperty("setop_type", setop_type);
	serializer.WriteProperty("left", *left);
	serializer.WriteProperty("right", *right);
}

unique_ptr<QueryNode> SetOperationNode::FormatDeserialize(FormatDeserializer &deserializer) {
	auto result = make_uniq<SetOperationNode>();
	deserializer.ReadProperty("setop_type", result->setop_type);
	deserializer.ReadProperty("left", result->left);
	deserializer.ReadProperty("right", result->right);
	return std::move(result);
}

void RecursiveCTENode::FormatSerialize(FormatSerializer &serializer) const {
	QueryNode::FormatSerialize(serializer);
	serializer.WriteProperty("cte_name", ctename);
	serializer.WriteProperty("union_all", union_all);
	serializer.WriteProperty("left", *left);
	serializer.WriteProperty("right", *right);
	serializer.WriteProperty("aliases", aliases);
}

unique_ptr<QueryNode> RecursiveCTENode::FormatDeserialize(FormatDeserializer &deserializer) {
	auto result = make_uniq<RecursiveCTENode>();
	deserializer.ReadProperty("cte_name", result->ctename);
	deserializer.ReadProperty("union_all", result->union_all);
	deserializer.ReadProperty("left", result->left);
	deserializer.ReadProperty("right", result->right);
	deserializer.ReadProperty("aliases", result->aliases);
	return std::move(result);
}

} // namespace duckdb
