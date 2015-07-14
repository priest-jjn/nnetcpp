#include "network.h"

#include <assert.h>

Network::Network(unsigned int inputs)
{
    _input_port.error = Vector::Zero(inputs);
    _input_port.value = Vector::Zero(inputs);
}

Network::~Network()
{
    for (AbstractNode *node : _nodes) {
        delete node;
    }
}

AbstractNode::Port* Network::inputPort()
{
    return &_input_port;
}

void Network::addNode(AbstractNode *node)
{
    _nodes.push_back(node);
}

Vector Network::predict(const Vector &input)
{
    assert(input.rows() == _input_port.value.rows());

    // Put the input in the input port, and propagate it through the network
    _input_port.value = input;

    for (AbstractNode *node : _nodes) {
        node->forward();
    }

    // Return the output of the last node
    AbstractNode *last = _nodes.back();

    return last->output()->value;
}

void Network::reset()
{
    // Call reset on all the nodes
    for (AbstractNode *node : _nodes) {
        node->reset();
    }
}

Float Network::setExpectedOutput(const Vector &output)
{
    AbstractNode *last = _nodes.back();

    return setError(last->output()->value - output);
}

Float Network::setError(const Vector &error)
{
    AbstractNode *last = _nodes.back();

    // Set the error of the last node
    assert(error.rows() == last->output()->error.rows());

    last->output()->error = error;

    // Backpropagate it
    for (int i=_nodes.size()-1; i>=0; --i) {
        _nodes[i]->backward();
    }

    return error.cwiseProduct(error).mean();
}

void Network::update()
{
    // Tell all the nodes to update their parameters, then discard the error signal
    for (AbstractNode *node : _nodes) {
        node->update();
        node->clearError();
    }
}

Float Network::trainSample(const Vector &input, const Vector &output)
{
    Float error;

    predict(input);
    error = setExpectedOutput(output);
    update();

    return error;
}