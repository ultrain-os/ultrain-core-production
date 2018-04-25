// const is_ethereum_address = (address) => {
//     if (!/^(0x)?[0-9a-f]{40}$/i.test(address)) {
//         // check if it has the basic requirements of an address
//         return false;
//     } else if (/^(0x)?[0-9a-f]{40}$/.test(address) || /^(0x)?[0-9A-F]{40}$/.test(address)) {
//         // If it's all small caps or all all caps, return true
//         return true;
//     } else {
//         // Otherwise check each case
//         return isChecksumAddress(address);
//     }
// };

const is_ethereum_address = (address) => {
    if (/^(0x)?[0-9a-fA-F]{40}$/.test(address)) {
        // If it's all small caps or all all caps, return true
        return true;
    }
};

const generate_account_name = ( name, index ) => {
  return ( is_ethereum_address( name ) )
    ? base32.encode( index.toString() ).replace(/=/g, "").toLowerCase()
    : name
}

const convert_to_hex = () => {
  if (s.length % 2 !== 0) {
    // odd number of characters
    s = '0' + s;
  }
  var bytes = [];
  for (var i = 0; i < s.length; i = i + 2) {
    bytes.push(parseInt(s.slice(i, i + 2), 16));
  }
  return bytes
}

const isChecksumAddress = (address) => {
    // Check each case
    address = address.replace('0x','');
    var addressHash = sha3_224(address.toLowerCase());
    for (var i = 0; i < 40; i++ ) {
        // the nth letter should be uppercase if the nth digit of casemap is 1
        if ((parseInt(addressHash[i], 16) > 7 && address[i].toUpperCase() !== address[i]) || (parseInt(addressHash[i], 16) <= 7 && address[i].toLowerCase() !== address[i])) {
            return false;
        }
    }
    return true;
};
