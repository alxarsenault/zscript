

function addCodeBlock(parentElement, section) {
  // If the subsection has a code example, add it
  if (section.code) {
    const codeBlock = document.createElement('pre');

    // Code element.
    const codeElement = document.createElement('code');
    codeElement.classList.add("zscript-code");
    codeElement.textContent = section.code;
    hljs.highlightElement(codeElement);
    // hljs.lineNumbersBlock(codeElement);
    codeBlock.appendChild(codeElement);

    parentElement.appendChild(codeBlock);
  }
}

function addGlossaryItem(sId, title, offset, glossaryList) {

  // Add subsections to the glossary.
  const subGlossaryItem = document.createElement('li');
  const subGlossaryLink = document.createElement('a');
  subGlossaryLink.href = "#" + sId;
  subGlossaryLink.textContent = title;

  // Indent to visually differentiate subsections.
  subGlossaryLink.style.marginLeft = offset;
  subGlossaryLink.addEventListener('click', function (e) {
    e.preventDefault();
    document.getElementById(sId).scrollIntoView({ behavior: 'smooth' });
  });
  subGlossaryItem.appendChild(subGlossaryLink);
  glossaryList.appendChild(subGlossaryItem);
}

//
function addSubSubSection(parentElement, section, index, subIndex, glossaryList) {
  // If the section has subsections, render them in the content area and add them to the glossary
  if (!(section.subsections && section.subsections.length > 0)) {
    return;
  }

  const subsectionList = document.createElement('ul'); // Create a list for subsections under the section

  section.subsections.forEach((subsection, subSubIndex) => {
    subSubIndex++;

    const sId = `section-${index}-subsection-${subIndex}-subsection-${subSubIndex}`;
    const title = `${index}.${subIndex}.${subSubIndex} - ` + subsection.title;

    // Add subsections to the glossary.
    addGlossaryItem(sId, title, '40px', glossaryList);

    // const subGlossaryItem = document.createElement('li');
    // const subGlossaryLink = document.createElement('a');
    // subGlossaryLink.href = "#" + sId;
    // subGlossaryLink.textContent = title;

    // // Indent to visually differentiate subsections.
    // subGlossaryLink.style.marginLeft = '40px';
    // subGlossaryLink.addEventListener('click', function (e) {
    //   e.preventDefault();
    //   document.getElementById(sId).scrollIntoView({ behavior: 'smooth' });
    // });
    // subGlossaryItem.appendChild(subGlossaryLink);
    // glossaryList.appendChild(subGlossaryItem);

    // Create the subsection content.
    const subsectionElement = document.createElement('section');
    subsectionElement.id = sId;

    const subsectionTitle = document.createElement('h4');
    subsectionTitle.textContent = title;
    subsectionElement.appendChild(subsectionTitle);

    if (subsection.content) {
      const subsectionContent = document.createElement('p');
      subsectionContent.innerHTML = subsection.content;
      subsectionElement.appendChild(subsectionContent);
    }

    // If the subsection has a code example, add it
    addCodeBlock(subsectionElement, subsection);

    parentElement.appendChild(subsectionElement);
  });
}

//
function addSubSection(parentElement, section, index, glossaryList) {
  // If the section has subsections, render them in the content area and add them to the glossary
  if (!(section.subsections && section.subsections.length > 0)) {
    return;
  }

  // Create a list for subsections under the section
  const subsectionList = document.createElement('ul');

  section.subsections.forEach((subsection, subIndex) => {
    subIndex++;

    const title = `${index}.${subIndex} - ` + subsection.title;
    const sId = `section-${index}-subsection-${subIndex}`;;

    // Add subsections to the glossary
    addGlossaryItem(sId, title, '20px', glossaryList);

    // const subGlossaryItem = document.createElement('li');
    // const subGlossaryLink = document.createElement('a');
    // subGlossaryLink.href = "#" + sId
    // subGlossaryLink.textContent = title;
    // subGlossaryLink.style.marginLeft = '20px'; // Indent to visually differentiate subsections
    // subGlossaryLink.addEventListener('click', function (e) {
    //   e.preventDefault();
    //   document.getElementById(sId).scrollIntoView({ behavior: 'smooth' });
    // });

    // subGlossaryItem.appendChild(subGlossaryLink);
    // glossaryList.appendChild(subGlossaryItem);

    // Create the subsection content
    const subsectionElement = document.createElement('section');
    subsectionElement.id = sId;

    const subsectionTitle = document.createElement('h3');
    subsectionTitle.textContent = title;
    subsectionElement.appendChild(subsectionTitle);

    if (subsection.content) {
      const subsectionContent = document.createElement('p');
      subsectionContent.innerHTML = subsection.content;
      subsectionElement.appendChild(subsectionContent);
    }

    // If the subsection has a code example, add it
    addCodeBlock(subsectionElement, subsection);

    addSubSubSection(subsectionElement, subsection, index, subIndex, glossaryList);
    parentElement.appendChild(subsectionElement);
  });

}

// Function to load and process the JSON file
function loadSections(sectionName) {
  const pages = pages_list;
  const nav = document.getElementById('pages-nav-list');

  let navSection;
  for (let i = 0; i < pages.length; i++) {
    const title = pages[i];
    const sectionElement = document.createElement('li');
    const pageLink = document.createElement('a');
    pageLink.href = pages[i] + ".html";

    if (title == "index") {
      pageLink.textContent = "Introduction";
    }
    else {
      pageLink.textContent = title.charAt(0).toUpperCase() + title.slice(1);
    }

   


    sectionElement.appendChild(pageLink);

    if (title == sectionName) {

      sectionElement.classList.add("selected-page");

      const sectionList = document.createElement('ul');
      sectionElement.appendChild(sectionList);
      navSection = sectionList;
    }
    nav.appendChild(sectionElement);
  }

  // console.log(sectionName)
  const sections = documentation_content[sectionName].sections;

  // Get references to the glossary and content areas
  const glossaryList = navSection;//document.getElementById('glossary-list');
  const contentArea = document.getElementById('content');

  // Loop through each section in the JSON data.
  sections.forEach((section, index) => {
    index++;

    const sId = `section-${index}`;
    const sTitle = `${index} - ` + section.title;

    addGlossaryItem(sId, sTitle, '0px', glossaryList);
    // Create glossary item for the section
    // const glossaryItem = document.createElement('li');
    // const glossaryLink = document.createElement('a');
    // glossaryLink.href = "#" + sId;
    // glossaryLink.textContent = sTitle;
    // glossaryLink.addEventListener('click', function (e) {
    //   e.preventDefault();
    //   document.getElementById(sId).scrollIntoView({ behavior: 'smooth' });
    // });
    // glossaryItem.appendChild(glossaryLink);
    // glossaryList.appendChild(glossaryItem);

    // Create the section content
    const sectionElement = document.createElement('section');
    sectionElement.id = sId;
    sectionElement.classList.add("zscript-top-section");

    const sectionTitle = document.createElement('h2');
    sectionTitle.textContent = sTitle;
    sectionElement.appendChild(sectionTitle);

    if (section.subtitle) {
      const sectionSubTitle = document.createElement('p');
      sectionSubTitle.innerHTML = section.subtitle;
      sectionSubTitle.classList.add("zscript-sub-title");

      sectionElement.appendChild(sectionSubTitle);
    }

    if (section.content) {
      const sectionContent = document.createElement('p');
      sectionContent.innerHTML = section.content;
      sectionElement.appendChild(sectionContent);
    }

    addCodeBlock(sectionElement, section);
    addSubSection(sectionElement, section, index, glossaryList);

    // Add the section to the main content area.
    contentArea.appendChild(sectionElement);
  });
}

// // Load sections when the page is loaded.
// window.onload = loadSections;