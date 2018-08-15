import React from 'react';
import { Modal, Input } from 'antd';
import ConfigContext from '../store/config';

class Header extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      visible: false,
      serverInput: props.server,
    }
  }

  showServerModal = () => {
    this.setState({ visible: true });
  }

  hideServerModal = () => {
    const { server } = this.props;
    this.setState({ visible: false, serverInput: server  });
  }
  
  submit = () => {
    const { serverInput } = this.state;
    this.props.update('server', serverInput);
  }

  updateServer = (e) => {
    const { value: serverInput } = e.target;
    this.setState({ serverInput });
  }

  render() {
    const { visible, serverInput } = this.state;
    return (
      <header className="App-header">
        <h1>Volume Visualization</h1>
        <div className="header-menu">
          <span onClick={this.showServerModal}>Server</span>
          <span>About</span>
        </div>
        <Modal
          title="Change Server URL"
          onOk={this.submit}
          onCancel={this.hideServerModal}
          visible={visible}
        >
          <Input value={serverInput} onChange={this.updateServer}></Input>
        </Modal>
      </header>
    );
  }
}

export default (props) => (
  <ConfigContext.Consumer>
    {({ server, update }) => (
      <Header {...props} server={server} update={update} />
    )}
  </ConfigContext.Consumer>
)