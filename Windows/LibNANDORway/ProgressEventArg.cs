namespace LibNANDORway {
    using System;

    public sealed class EventArg<T> : EventArgs {
        private readonly T _data;

        internal EventArg(T data) {
            _data = data;
        }

        public T Data {
            get { return _data; }
        }
    }

    public sealed class ProgressEventArg<TCurrent, TTotal> : EventArgs {
        private readonly TCurrent _current;
        private readonly TTotal _total;

        internal ProgressEventArg(TCurrent current, TTotal total) {
            _current = current;
            _total = total;
        }

        public TCurrent Current {
            get { return _current; }
        }

        public TTotal Total {
            get { return _total; }
        }
    }
}